import { resolveServiceOrigins, serviceUrl } from "../src/config/service-origin";

function assert(actual: unknown, expected: unknown, label: string): void {
  if (JSON.stringify(actual) !== JSON.stringify(expected)) {
    throw new Error(`${label}: expected ${JSON.stringify(expected)}, got ${JSON.stringify(actual)}`);
  }
}

function assertThrows(action: () => unknown, message: string, label: string): void {
  try {
    action();
  } catch (error) {
    if (error instanceof Error && error.message.includes(message)) return;
    throw new Error(`${label}: unexpected error ${String(error)}`);
  }
  throw new Error(`${label}: expected an error`);
}

assert(
  resolveServiceOrigins({}, "http://127.0.0.1:8787"),
  { api: "http://127.0.0.1:8787", events: "ws://127.0.0.1:8787" },
  "loopback mode stays same-origin",
);
assert(
  resolveServiceOrigins(
    {
      apiOrigin: "https://api.plywise.example/",
      eventOrigin: "wss://events.plywise.example",
    },
    "https://preview.plywise.example",
  ),
  { api: "https://api.plywise.example", events: "wss://events.plywise.example" },
  "hosted origins are normalized",
);
assert(
  resolveServiceOrigins(
    { apiOrigin: "https://api.plywise.example" },
    "https://preview.plywise.example",
  ).events,
  "wss://api.plywise.example",
  "event origin derives from configured API",
);
assert(
  serviceUrl("https://api.plywise.example", "/api/games?id=one"),
  "https://api.plywise.example/api/games?id=one",
  "service path remains root-relative",
);

assertThrows(
  () => resolveServiceOrigins({ apiOrigin: "https://user:pass@api.example" }, "https://app.example"),
  "cannot contain credentials",
  "credentials are rejected",
);
assertThrows(
  () => resolveServiceOrigins({ apiOrigin: "https://api.example/v1" }, "https://app.example"),
  "origin only",
  "paths are rejected",
);
assertThrows(
  () => resolveServiceOrigins({ apiOrigin: "javascript:alert(1)" }, "https://app.example"),
  "must use http: or https:",
  "unsafe API protocols are rejected",
);
assertThrows(
  () => resolveServiceOrigins({ eventOrigin: "ws://events.example" }, "https://app.example"),
  "cannot use WS",
  "mixed-content sockets are rejected",
);
assertThrows(
  () => resolveServiceOrigins({ apiOrigin: "http://api.example" }, "https://app.example"),
  "cannot use HTTP",
  "mixed-content API requests are rejected",
);
assertThrows(
  () => serviceUrl("https://api.example", "//attacker.example/path"),
  "root-relative",
  "protocol-relative paths are rejected",
);

console.log("service origin tests passed");
