import { resolveServiceOrigins, serviceUrl } from "./service-origin";

const origins = resolveServiceOrigins(
  {
    apiOrigin: import.meta.env.VITE_PLYWISE_API_ORIGIN,
    eventOrigin: import.meta.env.VITE_PLYWISE_EVENT_ORIGIN,
  },
  window.location.origin,
);

export function apiUrl(path: string): string {
  return serviceUrl(origins.api, path);
}

export function eventUrl(path: string): string {
  return serviceUrl(origins.events, path);
}
