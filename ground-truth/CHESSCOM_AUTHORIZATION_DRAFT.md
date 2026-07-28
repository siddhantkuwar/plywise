# Chess.com Integration Authorization Draft

Status: draft for the repository owner to review and submit. This is not a claim of authorization.

## Subject

Request to confirm a completed-game integration for Plywise

## Message

Hello,

I'm building Plywise, an open-source chess analysis project for reviewing completed games. The basic
product lets a user paste a completed Chess.com game URL or PGN, run Stockfish analysis, and review
the game in an independent interface.

I want to make sure the integration respects Chess.com's API, fair-play, data, and brand rules
before building a browser companion.

The proposed companion would:

- work only after a game is completed;
- require a clear user action;
- read only the current public completed-game URL or another signal you approve;
- send that URL to the user's Plywise session;
- resolve public game data through the integration path you approve;
- never access passwords, cookies, authentication tokens, chat, private data, or ongoing moves;
- never provide engine assistance during a live game;
- never copy Chess.com assets, UI, sounds, classifications, or branding;
- clearly state that Plywise is independent and not sponsored by Chess.com.

The initial extension would use a toolbar action or browser side panel. We would only consider
placing a small action near the completed board if you explicitly approve that behavior.

Plywise would use a descriptive API User-Agent, respect caching and rate limits, and let users
disconnect their public username and delete imported data. Single completed-game analysis would be
free.

Could you confirm:

1. whether this completed-game URL handoff and public-data analysis is permitted;
2. which API or page signal should be used to confirm that a game is complete;
3. whether a user-initiated button or side panel is acceptable;
4. whether you require an application registration, disclaimer, attribution, or additional review;
5. any rate, caching, retention, or branding rules we should add.

Repository: https://github.com/siddhantkuwar/plywise

Thank you for helping us build this responsibly.
