# Validation and limitations

## Automated tests

Screen classification, win/loss/draw outcomes, finite and unlimited repetition, session-only results, settings persistence and backup recovery, numeric bounds, duplicate opponents, formation exit, result deduplication, Rematch, Commander preservation, observation without input, stale results and paused-input rejection.

Localization tests check six-language resource completeness, stable Spanish recognition and saved language preferences.

## Local visual regression

Nine captures cover formation editing, two difficulty-menu variants, the team list, 5–0 and 4–0 scores, a score with a cursor, an unreadable transition, and a duel displaying “Derrota” that must not count as a final match loss. Game captures are not distributed. Offline tests require neither captures nor a running game.

## Live session: October 11, 2026

Version 0.1 completed two matches against the first Royal Academy entry on Heroic difficulty. It selected the opponent and difficulty from Chronicle, exited formation editing, enabled Commander and recorded two 4–0 wins. It used Rematch between games and stopped at 2/2 after returning to the menu. This took approximately eleven minutes.

The 0.2 interface preserves that navigation engine. Display languages do not change Spanish recognition rules. Draws and losses are covered by automated tests, not the live session above.

## Scope

**The game must be set to Spanish.** Recognition uses keyboard/mouse controls and a 1920 × 1080 reference. Not every opponent, difficulty, score, resolution or Windows font configuration has been tested.

Unlimited repetition is covered by engine tests; this is not a multi-day unattended reliability test. Raw game text and opponent names stay in Spanish. Windows builds and automated tests also run through GitHub Actions.
