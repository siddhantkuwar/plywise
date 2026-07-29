export interface ServiceOriginInput {
  apiOrigin?: string;
  eventOrigin?: string;
}

export interface ServiceOrigins {
  api: string;
  events: string;
}

function configuredOrigin(
  raw: string | undefined,
  variableName: string,
  protocols: readonly string[],
): string | null {
  const value = raw?.trim();
  if (!value) return null;

  let parsed: URL;
  try {
    parsed = new URL(value);
  } catch {
    throw new Error(`${variableName} must be an absolute URL.`);
  }

  if (!protocols.includes(parsed.protocol)) {
    throw new Error(`${variableName} must use ${protocols.join(" or ")}.`);
  }
  if (parsed.username || parsed.password) {
    throw new Error(`${variableName} cannot contain credentials.`);
  }
  if (parsed.pathname !== "/" || parsed.search || parsed.hash) {
    throw new Error(`${variableName} must contain an origin only, without a path, query, or fragment.`);
  }
  return parsed.origin;
}

function websocketOrigin(httpOrigin: string): string {
  const parsed = new URL(httpOrigin);
  parsed.protocol = parsed.protocol === "https:" ? "wss:" : "ws:";
  return parsed.origin;
}

export function resolveServiceOrigins(
  input: ServiceOriginInput,
  pageOrigin: string,
): ServiceOrigins {
  const page = configuredOrigin(pageOrigin, "window.location.origin", ["http:", "https:"]);
  if (!page) throw new Error("window.location.origin is required.");

  const api =
    configuredOrigin(input.apiOrigin, "VITE_PLYWISE_API_ORIGIN", ["http:", "https:"]) ?? page;
  const events =
    configuredOrigin(input.eventOrigin, "VITE_PLYWISE_EVENT_ORIGIN", ["ws:", "wss:"]) ??
    websocketOrigin(api);

  if (page.startsWith("https://") && api.startsWith("http://")) {
    throw new Error("VITE_PLYWISE_API_ORIGIN cannot use HTTP from an HTTPS page.");
  }
  if (page.startsWith("https://") && events.startsWith("ws://")) {
    throw new Error("VITE_PLYWISE_EVENT_ORIGIN cannot use WS from an HTTPS page.");
  }

  return { api, events };
}

export function serviceUrl(origin: string, path: string): string {
  if (!path.startsWith("/") || path.startsWith("//")) {
    throw new Error("Service paths must be root-relative.");
  }
  return `${origin}${path}`;
}
