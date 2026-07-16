#include "pct/import/chesscom_archive_client.hpp"

#include "pct/chess/pgn.hpp"
#include "pct/common/json.hpp"

#include <curl/curl.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <ctime>
#include <limits>
#include <memory>
#include <set>
#include <string_view>
#include <thread>

namespace pct::import {
namespace {

constexpr int max_retries = 2;
constexpr std::size_t max_archive_count = 600;
constexpr std::size_t max_header_bytes = 64U * 1024U;
constexpr std::size_t max_recent_archive_months = 4;

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

std::string trim(std::string_view value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0)
        value.remove_prefix(1);
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0)
        value.remove_suffix(1);
    return std::string(value);
}

bool valid_username(std::string_view username) {
    if (username.size() < 3 || username.size() > 25 ||
        std::isalnum(static_cast<unsigned char>(username.front())) == 0 ||
        std::isalnum(static_cast<unsigned char>(username.back())) == 0) {
        return false;
    }
    return std::all_of(username.begin(), username.end(), [](unsigned char character) {
        return std::isalnum(character) != 0 || character == '-' || character == '_';
    });
}

bool all_digits(std::string_view value) {
    return !value.empty() && std::all_of(value.begin(), value.end(), [](unsigned char character) {
               return std::isdigit(character) != 0;
           });
}

void validate_year_month(std::string_view year, std::string_view month) {
    if (year.size() != 4 || !all_digits(year))
        throw Error(ErrorCode::InvalidArgument, "Chess.com archive year must be four digits");
    if (month.size() != 2 || !all_digits(month) || month < "01" || month > "12")
        throw Error(ErrorCode::InvalidArgument, "Chess.com archive month must be 01 through 12");
}

void validate_game_id(std::string_view game_id) {
    if (game_id.empty() || game_id.size() > 32 || !all_digits(game_id))
        throw Error(ErrorCode::InvalidArgument, "Chess.com game identifier must be numeric");
}

std::optional<std::string_view> game_id_from_url(std::string_view url) {
    constexpr std::array<std::string_view, 2> prefixes = {
        "https://www.chess.com/game/live/", "https://www.chess.com/game/daily/"};
    for (const std::string_view prefix : prefixes) {
        if (url.starts_with(prefix)) {
            const std::string_view id = url.substr(prefix.size());
            if (id.size() <= 32 && all_digits(id))
                return id;
        }
    }
    return std::nullopt;
}

bool pgn_does_not_contradict(std::string_view pgn, std::string_view game_id) {
    try {
        const chess::Game game = chess::parse_pgn(pgn);
        const std::string site = game.tag("Site");
        const std::string link = game.tag("Link");
        const auto site_id = game_id_from_url(site);
        const auto link_id = game_id_from_url(link);
        if (site_id && *site_id != game_id)
            return false;
        if (link_id && *link_id != game_id)
            return false;
        // Current Chess.com archive PGNs commonly use the generic
        // `[Site "Chess.com"]` tag and omit Link. The archive entry's exact
        // JSON URL is therefore authoritative; embedded game URLs, when
        // present, are additional consistency checks.
        return true;
    } catch (const Error&) {
        return false;
    }
}

std::string header(const HttpHeaders& headers, std::string_view key) {
    const auto found = headers.find(std::string(key));
    return found == headers.end() ? std::string{} : found->second;
}

std::optional<std::chrono::seconds> parse_retry_after(const HttpHeaders& headers) {
    const std::string value = trim(header(headers, "retry-after"));
    if (value.empty())
        return std::nullopt;
    unsigned long long seconds = 0;
    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), seconds);
    if (parsed.ec == std::errc{} && parsed.ptr == value.data() + value.size()) {
        seconds = std::min<unsigned long long>(seconds, 60ULL);
        return std::chrono::seconds(seconds);
    }
    const std::time_t retry_at = curl_getdate(value.c_str(), nullptr);
    if (retry_at < 0)
        return std::nullopt;
    const std::time_t now = std::time(nullptr);
    const auto delay = retry_at > now ? static_cast<unsigned long long>(retry_at - now) : 0ULL;
    return std::chrono::seconds(std::min<unsigned long long>(delay, 60ULL));
}

void throw_cancelled() {
    throw ChessComClientError(ChessComFailure::Cancelled, ErrorCode::NetworkError,
                              "Chess.com request was cancelled");
}

void default_sleep(std::chrono::milliseconds duration, CancellationToken cancellation) {
    constexpr auto slice = std::chrono::milliseconds(25);
    while (duration.count() > 0) {
        if (cancellation.stop_requested())
            throw_cancelled();
        const auto current = std::min(duration, slice);
        std::this_thread::sleep_for(current);
        duration -= current;
    }
}

ErrorCode code_for_failure(ChessComFailure failure) {
    switch (failure) {
    case ChessComFailure::NotFound:
    case ChessComFailure::Gone:
        return ErrorCode::NotFound;
    case ChessComFailure::Timeout:
        return ErrorCode::Timeout;
    case ChessComFailure::InvalidResponse:
        return ErrorCode::ParseError;
    default:
        return ErrorCode::NetworkError;
    }
}

ChessComClientError status_error(const HttpResponse& response) {
    ChessComFailure failure = ChessComFailure::Transport;
    if (response.status == 404)
        failure = ChessComFailure::NotFound;
    else if (response.status == 410)
        failure = ChessComFailure::Gone;
    else if (response.status == 429)
        failure = ChessComFailure::RateLimited;
    else if (response.status >= 500 && response.status <= 599)
        failure = ChessComFailure::ServerError;
    return ChessComClientError(failure, code_for_failure(failure),
                               "Chess.com returned HTTP " + std::to_string(response.status),
                               response.status, parse_retry_after(response.headers));
}

void validate_https_host(std::string_view url) {
    constexpr std::string_view scheme = "https://";
    if (!url.starts_with(scheme))
        throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::NetworkError,
                                  "Chess.com transport only permits HTTPS");
    const std::size_t slash = url.find('/', scheme.size());
    const std::string_view authority = url.substr(
        scheme.size(), slash == std::string_view::npos ? url.size() - scheme.size()
                                                       : slash - scheme.size());
    const std::string host = lowercase(std::string(authority));
    if (host != "chess.com" && host != "www.chess.com" && host != "api.chess.com") {
        throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::NetworkError,
                                  "Chess.com redirect escaped the allowed hosts");
    }
}

bool is_pubapi_url(std::string_view url) {
    return url.starts_with("https://api.chess.com/pub/");
}

void validate_effective_endpoint(std::string_view requested, std::string_view effective) {
    validate_https_host(effective);
    if (is_pubapi_url(requested) && effective != requested)
        throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::NetworkError,
                                  "Chess.com PubAPI response changed the requested endpoint");
}

bool followed_redirect_status(long status) {
    return status == 301 || status == 302 || status == 303 || status == 307 || status == 308;
}

std::string resolve_redirect(std::string_view current, std::string_view location) {
    if (location.starts_with("https://")) {
        validate_https_host(location);
        return std::string(location);
    }
    if (!location.starts_with('/') || location.starts_with("//"))
        throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::NetworkError,
                                  "Chess.com returned an unsupported redirect");
    constexpr std::string_view scheme = "https://";
    const std::size_t slash = current.find('/', scheme.size());
    const std::string base(current.substr(0, slash));
    const std::string result = base + std::string(location);
    validate_https_host(result);
    return result;
}

struct CurlContext {
    std::string body;
    HttpHeaders headers;
    CancellationToken cancellation;
    std::size_t max_body_size{0};
    std::size_t header_bytes{0};
    bool oversized{false};
    bool headers_oversized{false};
};

std::size_t write_callback(char* data, std::size_t size, std::size_t count, void* user_data) {
    auto& context = *static_cast<CurlContext*>(user_data);
    if (size != 0 && count > std::numeric_limits<std::size_t>::max() / size) {
        context.oversized = true;
        return 0;
    }
    const std::size_t bytes = size * count;
    if (bytes > context.max_body_size - std::min(context.body.size(), context.max_body_size)) {
        context.oversized = true;
        return 0;
    }
    context.body.append(data, bytes);
    return bytes;
}

std::size_t header_callback(char* data, std::size_t size, std::size_t count, void* user_data) {
    auto& context = *static_cast<CurlContext*>(user_data);
    if (size != 0 && count > std::numeric_limits<std::size_t>::max() / size) {
        context.headers_oversized = true;
        return 0;
    }
    const std::size_t bytes = size * count;
    if (bytes > max_header_bytes - std::min(context.header_bytes, max_header_bytes)) {
        context.headers_oversized = true;
        return 0;
    }
    context.header_bytes += bytes;
    const std::string_view line(data, bytes);
    if (line.starts_with("HTTP/")) {
        context.headers.clear();
        return bytes;
    }
    const std::size_t colon = line.find(':');
    if (colon != std::string_view::npos) {
        std::string key = lowercase(trim(line.substr(0, colon)));
        std::string value = trim(line.substr(colon + 1));
        if (!key.empty()) {
            auto [item, inserted] = context.headers.emplace(std::move(key), value);
            if (!inserted)
                item->second += ", " + value;
        }
    }
    return bytes;
}

int progress_callback(void* user_data, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
    return static_cast<CurlContext*>(user_data)->cancellation.stop_requested() ? 1 : 0;
}

HttpResponse perform_once(const HttpRequest& request, std::string_view url) {
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
    if (!curl)
        throw ChessComClientError(ChessComFailure::Transport, ErrorCode::NetworkError,
                                  "failed to create HTTP client");

    CurlContext context{{}, {}, request.cancellation,
                        std::min(request.max_body_size, chesscom_max_body_size)};
    std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> request_headers(
        nullptr, curl_slist_free_all);
    for (const auto& [name, value] : request.headers) {
        if (name.find_first_of("\r\n:") != std::string::npos ||
            value.find_first_of("\r\n") != std::string::npos) {
            throw Error(ErrorCode::InvalidArgument, "invalid HTTP header");
        }
        curl_slist* appended =
            curl_slist_append(request_headers.get(), (name + ": " + value).c_str());
        if (appended == nullptr)
            throw ChessComClientError(ChessComFailure::Transport, ErrorCode::NetworkError,
                                      "failed to allocate HTTP headers");
        static_cast<void>(request_headers.release());
        request_headers.reset(appended);
    }

    const std::string current_url(url);
    curl_easy_setopt(curl.get(), CURLOPT_URL, current_url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT_MS, 5000L);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, 15000L);
    curl_easy_setopt(curl.get(), CURLOPT_PROTOCOLS_STR, "https");
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "personal-chess-tutor/0.3");
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, request_headers.get());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &context);
    curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA, &context);
    curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt(curl.get(), CURLOPT_XFERINFODATA, &context);

    const CURLcode result = curl_easy_perform(curl.get());
    if (context.oversized)
        throw ChessComClientError(ChessComFailure::BodyTooLarge, ErrorCode::NetworkError,
                                  "Chess.com response exceeded the 10 MiB limit");
    if (context.headers_oversized)
        throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::NetworkError,
                                  "Chess.com response headers were too large");
    if (result == CURLE_ABORTED_BY_CALLBACK && request.cancellation.stop_requested())
        throw_cancelled();
    if (result == CURLE_OPERATION_TIMEDOUT)
        throw ChessComClientError(ChessComFailure::Timeout, ErrorCode::Timeout,
                                  "Chess.com request timed out");
    if (result != CURLE_OK)
        throw ChessComClientError(ChessComFailure::Transport, ErrorCode::NetworkError,
                                  std::string("Chess.com request failed: ") +
                                      curl_easy_strerror(result));

    long status = 0;
    char* effective = nullptr;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status);
    curl_easy_getinfo(curl.get(), CURLINFO_EFFECTIVE_URL, &effective);
    return HttpResponse{status, std::move(context.headers),
                        effective == nullptr ? current_url : std::string(effective),
                        std::move(context.body)};
}

const json::Value& required_member(const json::Value::Object& object, std::string_view key,
                                   bool string) {
    const auto found = object.find(std::string(key));
    if (found == object.end() || (string && !found->second.is_string()))
        throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::ParseError,
                                  "Chess.com JSON has an invalid '" + std::string(key) + "'");
    return found->second;
}

std::string metadata_header(const HttpResponse& response, std::string_view name) {
    return header(response.headers, name);
}

} // namespace

ChessComClientError::ChessComClientError(
    ChessComFailure failure, ErrorCode code, std::string message, long status,
    std::optional<std::chrono::seconds> retry_after)
    : Error(code, std::move(message)), failure_(failure), status_(status),
      retry_after_(retry_after) {}

ChessComFailure ChessComClientError::failure() const noexcept { return failure_; }
long ChessComClientError::status() const noexcept { return status_; }
const std::optional<std::chrono::seconds>& ChessComClientError::retry_after() const noexcept {
    return retry_after_;
}

HttpResponse curl_http_get_with_hops(const HttpRequest& request,
                                     HttpHopTransport hop_transport) {
    static const int initialized = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (initialized != CURLE_OK)
        throw ChessComClientError(ChessComFailure::Transport, ErrorCode::NetworkError,
                                  "failed to initialize libcurl");
    if (request.max_body_size == 0 || request.max_body_size > chesscom_max_body_size)
        throw Error(ErrorCode::InvalidArgument, "invalid Chess.com response body limit");
    if (!hop_transport)
        throw Error(ErrorCode::InvalidArgument, "Chess.com hop transport is required");
    validate_https_host(request.url);
    if (request.cancellation.stop_requested())
        throw_cancelled();

    std::string current = request.url;
    for (int redirects = 0; redirects <= 3; ++redirects) {
        HttpResponse response = hop_transport(request, current);
        if (request.cancellation.stop_requested())
            throw_cancelled();
        if (response.effective_url.empty())
            response.effective_url = current;
        validate_effective_endpoint(current, response.effective_url);
        if (!followed_redirect_status(response.status))
            return response;
        const std::string location = header(response.headers, "location");
        if (location.empty() || redirects == 3)
            throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::NetworkError,
                                      "Chess.com returned an invalid redirect chain");
        const std::string target = resolve_redirect(current, location);
        if (is_pubapi_url(request.url) && target != request.url)
            throw ChessComClientError(ChessComFailure::InvalidResponse,
                                      ErrorCode::NetworkError,
                                      "Chess.com PubAPI redirect changed the requested endpoint");
        current = target;
    }
    throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::NetworkError,
                              "Chess.com returned too many redirects");
}

HttpResponse curl_http_get(const HttpRequest& request) {
    return curl_http_get_with_hops(request, perform_once);
}

ChessComArchiveClient::ChessComArchiveClient(HttpTransport transport, RetrySleeper sleeper)
    : transport_(transport ? std::move(transport)
                           : HttpTransport([](const HttpRequest& request) {
                                 return curl_http_get(request);
                             })),
      sleeper_(sleeper ? std::move(sleeper) : default_sleep) {}

std::string ChessComArchiveClient::archive_index_url(std::string_view username) {
    if (!valid_username(username))
        throw Error(ErrorCode::InvalidArgument, "invalid Chess.com username");
    return "https://api.chess.com/pub/player/" + std::string(username) +
           "/games/archives";
}

std::string ChessComArchiveClient::monthly_archive_url(std::string_view username,
                                                        std::string_view year,
                                                        std::string_view month) {
    validate_year_month(year, month);
    const std::string index = archive_index_url(username);
    return index.substr(0, index.size() - std::string("archives").size()) + std::string(year) +
           "/" + std::string(month);
}

void ChessComArchiveClient::validate_archive_url(std::string_view url,
                                                  std::string_view username) {
    if (!valid_username(username))
        throw Error(ErrorCode::InvalidArgument, "invalid Chess.com username");
    constexpr std::string_view prefix = "https://api.chess.com/pub/player/";
    if (!url.starts_with(prefix) || url.find_first_of("?#@%\\") != std::string_view::npos)
        throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::ParseError,
                                  "Chess.com returned an invalid archive URL");
    const std::string expected_user = lowercase(std::string(username));
    const std::string_view suffix = url.substr(prefix.size());
    const std::size_t slash = suffix.find('/');
    if (slash == std::string_view::npos || lowercase(std::string(suffix.substr(0, slash))) !=
                                               expected_user) {
        throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::ParseError,
                                  "Chess.com archive URL did not match the requested player");
    }
    const std::string_view path = suffix.substr(slash);
    constexpr std::string_view games = "/games/";
    if (!path.starts_with(games))
        throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::ParseError,
                                  "Chess.com returned an invalid archive path");
    const std::string_view date = path.substr(games.size());
    if (date.size() != 7 || date[4] != '/')
        throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::ParseError,
                                  "Chess.com returned an invalid archive month");
    validate_year_month(date.substr(0, 4), date.substr(5, 2));
}

HttpResponse ChessComArchiveClient::request(std::string url,
                                             const ConditionalHeaders& conditional,
                                             CancellationToken cancellation) const {
    HttpRequest request{std::move(url), {}, chesscom_max_body_size, cancellation};
    if (conditional.etag.find_first_of("\r\n") != std::string::npos ||
        conditional.last_modified.find_first_of("\r\n") != std::string::npos) {
        throw Error(ErrorCode::InvalidArgument, "invalid conditional HTTP header");
    }
    if (!conditional.etag.empty())
        request.headers.emplace("If-None-Match", conditional.etag);
    if (!conditional.last_modified.empty())
        request.headers.emplace("If-Modified-Since", conditional.last_modified);

    for (int attempt = 0;; ++attempt) {
        if (cancellation.stop_requested())
            throw_cancelled();
        try {
            HttpResponse response = transport_(request);
            if (cancellation.stop_requested())
                throw_cancelled();
            if (response.body.size() > request.max_body_size)
                throw ChessComClientError(ChessComFailure::BodyTooLarge, ErrorCode::NetworkError,
                                          "Chess.com response exceeded the 10 MiB limit");
            if (response.effective_url.empty())
                response.effective_url = request.url;
            validate_effective_endpoint(request.url, response.effective_url);
            if (response.status == 200 || response.status == 304)
                return response;
            ChessComClientError error = status_error(response);
            const bool retryable = response.status == 429 ||
                                   (response.status >= 500 && response.status <= 599);
            if (!retryable || attempt >= max_retries)
                throw error;
            const auto wait = error.retry_after()
                                  ? std::chrono::duration_cast<std::chrono::milliseconds>(
                                        *error.retry_after())
                                  : std::chrono::milliseconds(250 * (1 << attempt));
            sleeper_(wait, cancellation);
        } catch (const ChessComClientError& error) {
            if (error.failure() != ChessComFailure::Timeout || attempt >= max_retries)
                throw;
            sleeper_(std::chrono::milliseconds(250 * (1 << attempt)), cancellation);
        }
    }
}

ChessComArchiveIndex ChessComArchiveClient::discover(
    std::string_view username, const ConditionalHeaders& conditional,
    CancellationToken cancellation) const {
    const std::string endpoint = archive_index_url(username);
    const HttpResponse response = request(endpoint, conditional, cancellation);
    ChessComArchiveIndex result{response.status == 304, {}, metadata_header(response, "etag"),
                                metadata_header(response, "last-modified"),
                                response.effective_url};
    if (result.not_modified)
        return result;
    try {
        const json::Value document = json::parse(response.body);
        const auto& root = document.as_object();
        const auto& archives = required_member(root, "archives", false);
        if (!archives.is_array() || archives.as_array().size() > max_archive_count)
            throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::ParseError,
                                      "Chess.com JSON has an invalid 'archives'");
        for (const auto& item : archives.as_array()) {
            if (!item.is_string())
                throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::ParseError,
                                          "Chess.com archive URL must be a string");
            validate_archive_url(item.as_string(), username);
            result.archives.push_back(item.as_string());
        }
    } catch (const ChessComClientError&) {
        throw;
    } catch (const Error& error) {
        throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::ParseError,
                                  std::string("invalid Chess.com archive JSON: ") + error.what());
    }
    return result;
}

ChessComMonthlyArchive ChessComArchiveClient::fetch_month(
    std::string_view username, std::string_view year, std::string_view month,
    const ConditionalHeaders& conditional, CancellationToken cancellation) const {
    const std::string endpoint = monthly_archive_url(username, year, month);
    const HttpResponse response = request(endpoint, conditional, cancellation);
    ChessComMonthlyArchive result{response.status == 304, {}, metadata_header(response, "etag"),
                                  metadata_header(response, "last-modified"),
                                  response.effective_url};
    if (result.not_modified)
        return result;
    try {
        const json::Value document = json::parse(response.body);
        const auto& root = document.as_object();
        const auto& games = required_member(root, "games", false);
        if (!games.is_array() || games.as_array().size() > 100000U)
            throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::ParseError,
                                      "Chess.com JSON has an invalid 'games'");
        for (const auto& item : games.as_array()) {
            if (!item.is_object())
                throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::ParseError,
                                          "Chess.com archive game must be an object");
            const auto& object = item.as_object();
            ChessComArchiveGame game;
            game.url = required_member(object, "url", true).as_string();
            game.pgn = required_member(object, "pgn", true).as_string();
            if (!game_id_from_url(game.url))
                throw ChessComClientError(ChessComFailure::InvalidResponse,
                                          ErrorCode::ParseError,
                                          "Chess.com archive game URL is invalid");
            const auto time_class = object.find("time_class");
            if (time_class != object.end()) {
                if (!time_class->second.is_string())
                    throw ChessComClientError(ChessComFailure::InvalidResponse,
                                              ErrorCode::ParseError,
                                              "Chess.com time_class must be a string");
                game.time_class = time_class->second.as_string();
            }
            result.games.push_back(std::move(game));
        }
    } catch (const ChessComClientError&) {
        throw;
    } catch (const Error& error) {
        throw ChessComClientError(ChessComFailure::InvalidResponse, ErrorCode::ParseError,
                                  std::string("invalid Chess.com monthly JSON: ") + error.what());
    }
    return result;
}

ChessComArchiveGame ChessComArchiveClient::find_game(
    std::string_view username, std::string_view game_id, std::string_view year,
    std::string_view month, CancellationToken cancellation) const {
    validate_game_id(game_id);
    std::vector<std::string> archive_urls;
    if (!year.empty() || !month.empty()) {
        archive_urls.push_back(monthly_archive_url(username, year, month));
    } else {
        ChessComArchiveIndex index = discover(username, {}, cancellation);
        std::set<std::string> seen_months;
        for (auto item = index.archives.rbegin(); item != index.archives.rend() &&
                                                archive_urls.size() < max_recent_archive_months;
             ++item) {
            validate_archive_url(*item, username);
            const std::string month_key = item->substr(item->size() - 7);
            if (seen_months.emplace(month_key).second)
                archive_urls.push_back(*item);
        }
    }

    for (const std::string& archive_url : archive_urls) {
        validate_archive_url(archive_url, username);
        const std::string_view date = std::string_view(archive_url).substr(archive_url.size() - 7);
        ChessComMonthlyArchive page =
            fetch_month(username, date.substr(0, 4), date.substr(5, 2), {}, cancellation);
        for (const ChessComArchiveGame& game : page.games) {
            if (game.url == "https://www.chess.com/game/live/" + std::string(game_id) ||
                game.url == "https://www.chess.com/game/daily/" + std::string(game_id)) {
                if (!pgn_does_not_contradict(game.pgn, game_id))
                    throw ChessComClientError(ChessComFailure::InvalidResponse,
                                              ErrorCode::ParseError,
                                              "Chess.com archive PGN did not match its game URL");
                return game;
            }
        }
    }
    throw ChessComClientError(ChessComFailure::NotFound, ErrorCode::NotFound,
                              "game was not found in the Chess.com archive");
}

} // namespace pct::import
