<p align="center">
  <img src="docs/logo.png" alt="Zzen logo" width="64" height="64">
</p>

# Zzen · Free

A native C++ desktop app for repeating **Chronicle matches** in Inazuma Eleven: Victory Road with Commander mode and Rematch.

> **The game must be set to Spanish.** Changing Zzen's display language does not change this requirement. Recognition is designed for the Spanish game interface.

**[Download for Windows](https://github.com/kazah-png/Zzen/releases/latest)**

**[Join the Zzen Discord community](https://discord.gg/UJxYEmPra)** — Get help, report bugs, share feedback and follow development. Public repository commits appear in **#github-updates**; release news is shared in **#announcements**. The server also has a dedicated area for the upcoming Premium edition.

![Zzen desktop interface](docs/interface.png)

## Features

- Match selection by category, opponent, occurrence and difficulty.
- A fixed match count or unlimited repetition.
- A catalog of 147 entries across nine categories, including Heroes' Victory Road and Gate/Gate+ battles, refreshable from the game.
- Start, pause, resume and **F8** to stop.
- Session results and win/loss counters.
- Diagnostics with screenshots, recognized text and recent actions.
- Spanish, English, Japanese, Simplified Chinese, French and Hindi display languages.

Free keeps results only for the current session. Item recognition and persistent reward tracking belong to the separate Premium edition.

## Getting started

1. Download the Windows x64 ZIP and **extract the entire folder**.
2. Open `Zzen.exe`. No installation, account or activation key is needed.
3. **Set the game to Spanish**, use keyboard/mouse controls and open a **Chronicle** route.
4. Choose the opponent, difficulty and match count. **Occurrence** distinguishes repeated opponents.
5. Select **Start session**. Zzen finds the opponent, enables Commander and repeats with Rematch.

Keep the game visible and focused. Switching windows pauses Zzen; **Resume** continues. **F8** stops it. Only one Zzen edition can run at a time.

## Prepare a strong team

**For the best farming results, use a strong, well-equipped in-game team suited to your chosen opponent and difficulty.** Zzen repeats matches in Commander mode; your squad, equipment, tactics and team passives still determine how well it performs. Play a trial match in Commander mode before starting a long session, and choose a difficulty your team can beat consistently. No lineup guarantees every win.

### Example: the author's level 99 squad

The screenshots below show the author's level **99** squad as a reference, not a required roster. Formation, team configuration and tactic names below are English translations of the Spanish labels in the screenshots.

| Setting | Example |
|---|---|
| Team level | 99 |
| Formation | 5-4-1 Double Pivot |
| Team configuration | Justice |
| Displayed team AT / DF | 14,665 / 13,701 |
| Coach | Alejandro |
| Forward | Shadow Cimmerian |
| Midfielders | Xene, Paolo Bianchi, Mehr, Riccardo Di Rigo |
| Defenders | Bai Long, Axel Blaze, Nathan Swift, Jack Wallside, Goldie Lemmon |
| Goalkeeper | Mark Evans |
| Tactics | Flame Fortress, Unyielding Suppression, Shell Formation |

![Author's level 99 squad and 5-4-1 formation, with personal names blurred](docs/example-team.png)

The accompanying passive setup includes **+90% tension when winning a focus or contest**, **+10% wall DF until the team is fouled**, and a **+8% team shot AT bonus for 30 seconds after regaining possession without a direct catch**. These are the values shown in this example; use the screenshots to compare your own setup, and check its performance against the actual match conditions.

<details>
<summary>View the example team's passive bonuses</summary>

![Passive bonuses for the author's Justice team configuration](docs/example-team-passives.png)

</details>

## App language versus game language

Use the top-right language selector: **Español, English, 日本語, 简体中文, Français or हिन्दी**. Your choice is saved immediately.

Opponent names, catalog entries and raw OCR evidence remain in Spanish, as read from the game. **The required game language is always Spanish**, regardless of the app language.

## Requirements and limitations

Windows x64 with Windows text recognition available. Spanish game UI with keyboard/mouse controls; visual reference **1920 × 1080**, 16:9. Required difficulties must be unlocked in your save. The catalog does not unlock matches.

Recognition may need adjustment when the game interface or layout changes. See [validation and limitations](docs/VALIDATION.md).

Zzen uses screenshots and normal Windows input. It does not modify game files, inject code, read process memory or alter anti-cheat. This is an unofficial project, not affiliated with LEVEL5.

## Troubleshooting

Open **Diagnostics**. **Observe game** analyzes the screen without sending inputs. **Save diagnostics** saves the capture and activity log locally.

A missed or unreadable score is marked **Pending**, with evidence saved locally, and farming continues. A visible final score is retried for up to 20 seconds before it is left pending. Pending matches count toward the session's match limit but never count as a win or loss. Review the evidence before using **Review result**. Free results reset on a new session or when closing the app. Unrecognized screens and loss of game focus can still pause the session.

## Local data

There are no accounts or telemetry. Files stay beside the executable.

| Folder | Contents |
|---|---|
| `data` | Settings, catalog and activity log |
| `debug` | Diagnostics explicitly saved by the user |
| `captures` | Evidence for unreadable results |
| `evidence` | Captures from catalog scanning |

JSON writes are atomic and keep a `.bak` backup. Generated data is excluded from this repository. Review captures before sharing them in an issue.

## Build and test

Install Visual Studio Build Tools with **Desktop development with C++**, a Windows SDK with C++/WinRT, and PowerShell.

```powershell
./build.ps1 -Test
```

This builds `dist/Zzen/Zzen.exe` and runs classification, storage, localization and engine tests. Engine tests replace input injection with in-memory events and **do not control the game**.

`CMakeLists.txt` is also provided for MSVC. Local visual tests require private game captures, which are not included:

```powershell
./tests/visual.ps1 -Fixtures C:/path/to/test-captures
```

## Source layout

`core.hpp`: classification; `vision.hpp`: capture and OCR; `zzen.cpp`: engine; `catalog_engine.inc`: navigation; `storage.hpp`: settings; `ui.hpp`: Win32 interface; `i18n.hpp`: display languages.

## License

Zzen Free is licensed under the [MIT License](LICENSE). This license covers this repository's code and documentation; it does not grant rights to the game, its artwork, names or trademarks. The separate Premium edition is not included in this repository.
