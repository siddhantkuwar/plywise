#pragma once

#include "pct/common/cancellation.hpp"
#include "pct/common/error.hpp"

#include <chrono>
#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace pct::import {

inline constexpr std::size_t chesscom_max_body_size = 10U * 1024U * 1024U;

using HttpHeaders = std::map<std::string, std::string>;

struct HttpRequest {
    std::string url;
    HttpHeaders headers;
    std::size_t max_body_size{chesscom_max_body_size};
    CancellationToken cancellation;
};

struct HttpResponse {
    long status{0};
    HttpHeaders headers;
    std::string effective_url;
    std::string body;
};

using HttpTransport = std::function<HttpResponse(const HttpRequest&)>;
// Injectable one-hop executor used to deterministically verify curl redirect policy.
using HttpHopTransport =
    std::function<HttpResponse(const HttpRequest&, std::string_view)>;
using RetrySleeper =
    std::function<void(std::chrono::milliseconds, CancellationToken)>;

enum class ChessComFailure {
    NotFound,
    Gone,
    RateLimited,
    ServerError,
    Timeout,
    BodyTooLarge,
    Cancelled,
    Transport,
    InvalidResponse,
};

class ChessComClientError : public Error {
  public:
    ChessComClientError(ChessComFailure failure, ErrorCode code, std::string message,
                        long status = 0,
                        std::optional<std::chrono::seconds> retry_after = std::nullopt);

    [[nodiscard]] ChessComFailure failure() const noexcept;
    [[nodiscard]] long status() const noexcept;
    [[nodiscard]] const std::optional<std::chrono::seconds>& retry_after() const noexcept;

  private:
    ChessComFailure failure_;
    long status_;
    std::optional<std::chrono::seconds> retry_after_;
};

struct ConditionalHeaders {
    std::string etag;
    std::string last_modified;
};

struct ChessComArchiveIndex {
    bool not_modified{false};
    std::vector<std::string> archives;
    std::string etag;
    std::string last_modified;
    std::string effective_url;
};

struct ChessComArchiveGame {
    std::string url;
    std::string pgn;
    // Kept as the PubAPI supplied string so new/unknown classes remain lossless.
    std::string time_class;
};

struct ChessComMonthlyArchive {
    bool not_modified{false};
    std::vector<ChessComArchiveGame> games;
    std::string etag;
    std::string last_modified;
    std::string effective_url;
};

class ChessComArchiveClient {
  public:
    explicit ChessComArchiveClient(HttpTransport transport = {}, RetrySleeper sleeper = {});

    [[nodiscard]] ChessComArchiveIndex
    discover(std::string_view username, const ConditionalHeaders& conditional = {},
             CancellationToken cancellation = {}) const;

    [[nodiscard]] ChessComMonthlyArchive
    fetch_month(std::string_view username, std::string_view year, std::string_view month,
                const ConditionalHeaders& conditional = {},
                CancellationToken cancellation = {}) const;

    [[nodiscard]] ChessComArchiveGame
    find_game(std::string_view username, std::string_view game_id,
              std::string_view year = {}, std::string_view month = {},
              CancellationToken cancellation = {}) const;

    [[nodiscard]] static std::string archive_index_url(std::string_view username);
    [[nodiscard]] static std::string monthly_archive_url(std::string_view username,
                                                         std::string_view year,
                                                         std::string_view month);
    static void validate_archive_url(std::string_view url, std::string_view username);

  private:
    [[nodiscard]] HttpResponse request(std::string url, const ConditionalHeaders& conditional,
                                       CancellationToken cancellation) const;

    HttpTransport transport_;
    RetrySleeper sleeper_;
};

[[nodiscard]] HttpResponse curl_http_get(const HttpRequest& request);
[[nodiscard]] HttpResponse curl_http_get_with_hops(const HttpRequest& request,
                                                   HttpHopTransport hop_transport);

} // namespace pct::import
