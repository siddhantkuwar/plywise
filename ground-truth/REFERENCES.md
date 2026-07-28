# Research References

Checked July 2026.

## OpenAI / Codex

- Custom instructions with AGENTS.md: https://developers.openai.com/codex/agent-configuration/agents-md
- Build skills: https://developers.openai.com/codex/build-skills
- Designing delightful frontends with GPT-5.4: https://developers.openai.com/blog/designing-delightful-frontends-with-gpt-5.4
- Build responsive front-end designs: https://developers.openai.com/codex/use-cases/frontend-designs
- Codex prompting guide: https://developers.openai.com/cookbook/examples/gpt-5/codex_prompting_guide
- Best practices: https://developers.openai.com/codex/learn/best-practices

Applied principles:

- Root `AGENTS.md` supplies durable repository guidance.
- Skills package reusable instructions, references, and optional scripts.
- Frontend prompts need explicit visual constraints, real references, existing-component reuse, multiple states, and browser verification.
- Long work benefits from staged milestones and acceptance criteria.

## Chess.com

- PubAPI: https://support.chess.com/en/articles/9650547-what-is-the-pubapi-and-how-do-i-use-it
- PGN export: https://support.chess.com/en/articles/8705305-how-do-i-get-a-pgn-of-my-game
- Move classification: https://support.chess.com/en/articles/8572705-how-are-moves-classified-what-is-a-blunder-or-brilliant-etc
- Game Review: https://support.chess.com/en/articles/8584089-how-does-game-review-work
- Brand resources: https://www.chess.com/article/view/chess-com-brand-resources
- User agreement: https://www.chess.com/legal/user-agreement

Applied principles:

- Use the public read-only API and respect caching/rate behavior.
- Implement an independent classifier.
- Create original icons, colors, board treatment, copy, and layout.
- Public game data and protected product UI are separate matters.
- The browser companion requires written clarification before implementation.

## Web engine and extension

- Stockfish.js: https://github.com/nmrugg/stockfish.js/
- Stockfish license: https://stockfishchess.org/about/
- Chrome extension policies: https://developer.chrome.com/docs/webstore/program-policies/policies
- Chrome activeTab: https://developer.chrome.com/docs/extensions/develop/concepts/activeTab
- Chrome side panel: https://developer.chrome.com/docs/extensions/reference/api/sidePanel

Applied principles:

- Run free engine work in a browser worker where supported.
- Record engine version and configuration.
- Treat browser observations as untrusted until C++ validation.
- Use one narrow extension purpose and the least permissions.
- Do not load remotely hosted extension code.

## Hosting research

- Cloudflare Pages: https://developers.cloudflare.com/pages/
- Cloudflare Workers limits: https://developers.cloudflare.com/workers/platform/limits/
- Supabase pricing and free-tier limits: https://supabase.com/pricing
- GitHub Student Developer Pack: https://education.github.com/pack

Applied principles:

- Static hosting can begin free.
- Edge request handlers are not a substitute for sustained Stockfish CPU.
- Free service limits are prototype constraints, not a durable scaling plan.
- Use a provider subdomain until a stable project domain is justified.

## Legal note

This package is an engineering/product specification, not legal advice. Obtain qualified review for
license compatibility, Stockfish distribution, branding, data use, and extension behavior when
uncertainty remains.
