/// <reference types="vite/client" />

interface ImportMetaEnv {
  readonly VITE_PLYWISE_API_ORIGIN?: string;
  readonly VITE_PLYWISE_EVENT_ORIGIN?: string;
}

interface ImportMeta {
  readonly env: ImportMetaEnv;
}
