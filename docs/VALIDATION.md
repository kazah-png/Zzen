# Validation and limitations

## Automated tests

Screen classification, win/loss/draw outcomes, finite and unlimited repetition, session-only results, settings persistence and backup recovery, numeric bounds, duplicate opponents, formation exit, result deduplication, Rematch, Commander preservation, observation without input, stale results and paused-input rejection.

Localization tests check six-language resource completeness, stable Spanish recognition and saved language preferences.

Version 0.2.1 adds regression coverage for accented Spanish capitals in the default C locale, uppercase reward headings, a missing results heading after a completed match, and alternating OCR frames without resetting the screen timeout or counting the match twice.

Version 0.2.2 covers a missed score followed by Rewards and Rematch, finite and unlimited sessions with pending outcomes, bounded retries for unreadable final scores, recovery when only the final-score footer is readable, and missing action buttons. A pending result never becomes a win automatically. Rematch state and completion change only after an input was actually sent.

## Local visual regression

Nine captures cover formation editing, two difficulty-menu variants, the team list, 5–0 and 4–0 scores, a score with a cursor, an unreadable transition, and a duel displaying “Derrota” that must not count as a final match loss. Game captures are not distributed. Offline tests require neither captures nor a running game.

For the 0.2.1 fix, the privately saved October 11 reward-screen diagnostic was also replayed through Windows OCR and correctly classified as Rewards. Pass its image with `-RewardsCapture C:/path/to/pantalla.png` to include this regression in the visual test script. The new fix was checked against the saved capture and simulated engine transitions; the live session below predates it.

The 0.2.2 investigation used a second private diagnostic from a Zeus session that paused at Rematch with an unknown score. Its screenshot is included locally through `-RematchCapture C:/path/to/pantalla.png`. Both fixes are validated with saved screens and simulated engine input; this is not a claim of a new full live-session test.

## Live session: October 11, 2026

Version 0.1 completed two matches against the first Royal Academy entry on Heroic difficulty. It selected the opponent and difficulty from Chronicle, exited formation editing, enabled Commander and recorded two 4–0 wins. It used Rematch between games and stopped at 2/2 after returning to the menu. This took approximately eleven minutes.

The 0.2 interface preserves that navigation engine. Display languages do not change Spanish recognition rules. Draws and losses are covered by automated tests, not the live session above.

## Scope

**The game must be set to Spanish.** Recognition uses keyboard/mouse controls and a 1920 × 1080 reference. Not every opponent, difficulty, score, resolution or Windows font configuration has been tested.

Unlimited repetition is covered by engine tests; this is not a multi-day unattended reliability test. Raw game text and opponent names stay in Spanish. Windows builds and automated tests also run through GitHub Actions.
