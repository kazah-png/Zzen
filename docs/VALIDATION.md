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

## Chronicle start regression: October 12, 2026

The shared opponent-selection routine required the first row to be highlighted before it considered the list to be at the top. A stationary mouse could keep the fourth row highlighted while the scrollbar was already at the beginning, causing repeated upward scrolling.

The routine now checks the scrollbar position over two readable frames, independently of the highlighted row. It limits unsuccessful scroll attempts to 24. Empty OCR pages cannot confirm the beginning. Regression tests cover a hovered fourth row, a lower page with its first row highlighted, missing scrollbar, bounded retries and repeated opponent occurrences.

Both Free and Premium passed the engine test against the actual private screenshot and Windows OCR: Brain is selected while Wild is highlighted. All four Free suites and all nine Premium suites passed. The source change is shared; Premium-only code and protection remain outside this repository.
The live check also exposed an immediate pause when the pre-match opponent heading was missed by full-screen OCR. Both editions now read an enlarged opponent banner and allow 12 seconds for verification before pausing; no start input is sent without the configured opponent name. Tests reject missing names, verify bounded waiting and pass against the actual Brain pre-match screenshot.
Live Premium verification: selected Brain and Heroic from the list; the final build verified Brain, confirmed the match, advanced presentation and exited formation editing to reach kickoff. Free uses the same fixes and passed all four suites plus both actual screenshot regressions; a full live Free match was not repeated.

## Chronicle expansion: October 13, 2026

Released as **0.3.0**. The four automated suites passed on the release source, including the optional private Royal Academy screenshot regression. The distributed ZIP contains only the application, public catalog, license and documentation; profiles, diagnostics, game captures and Premium data are excluded.

The shared catalog now includes the ninth category, Heroes' Victory Road, and its five gate opponents (147 entries in total). Free and Premium use the same category spacing, short-list handling, verified gate-menu anchors and Puerta/Puerta+ difficulty IDs. Existing difficulty IDs and saved profiles remain compatible.

Arrow navigation uses extended virtual-key events; ordinary gameplay shortcuts retain their existing scan-code handling. Tests cover the input flags, missing difficulty labels, gate selection, route-map continuation and all previous result/rematch regressions. All four Free suites pass. A full live Free match has not been repeated for this catalog update.

The actual Royal Academy six-difficulty screenshot is also checked: all six menu rows must be located at their observed positions, including labels missed by OCR. The fallback requires matching squad, save, legendary and unusual-item anchors. All four Free suites pass with this fixture.

The live catalog refresh found eleven additional Orion entries. It also exposed a narrower heading boundary that rejected the long Heroes' Victory Road title, and prefix matching that could confuse Inazuma Eleven with its sequels when refreshing the selector. Both editions now accept the observed heading position and select exact category/opponent names. The catalog contains 147 match occurrences; repeated opponents in different positions remain distinct matches.
