# Plywise web

The React app builds as static files and still works with the local C++ runtime by default.

## Run locally

```sh
npm ci
npm run dev
```

An empty configuration uses the page origin for `/api/*` and derives `/ws` from it, preserving the
existing loopback flow. For a separately hosted C++ service, copy `.env.example` to `.env.local`
and set:

- `VITE_PLYWISE_API_ORIGIN` to an `http://` or `https://` origin
- `VITE_PLYWISE_EVENT_ORIGIN` to a `ws://` or `wss://` origin

Both values must be origins only: no paths, credentials, queries, or fragments. HTTPS pages require
HTTPS and WSS services. Every `VITE_` variable is public, so the build rejects unrecognized public
variables and none may contain server credentials.

## Hosting boundary

`netlify.toml` builds `web/`, supplies SPA fallback routing, and provides pull-request deploy
previews once the GitHub repository is linked to a Netlify project. The build emits security headers
whose `connect-src` policy contains only the configured API and event origins.

The current C++ reference runtime deliberately accepts loopback origins only. Before a hosted API is
connected, its deployment must explicitly allow the production and preview frontend origins,
answer CORS preflight requests, validate WebSocket `Origin`, and keep credentials server-side.
Cross-origin isolation is not enabled until the selected browser Stockfish build proves it is
required.
