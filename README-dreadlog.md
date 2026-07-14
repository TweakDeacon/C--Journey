# 🩸 DreadLog — Horror Short Film Notes

A single-file app for taking timestamped notes on horror short films and ranking them. No install, no server, no account.

## How to use it

1. Download `horror-notes.html` and double-click it (opens in any modern browser).
2. Click **＋ New Film**, give it a title.
3. Load the film:
   - **Local file** — click the button or drag a video file onto the player, or
   - **YouTube** — paste the URL and click *Load YouTube* (needs internet).
4. Scrub to any moment and hit **⏱ Grab Timestamp** (or press `T`). The current time is pinned to a new note. Click any timestamp chip later to jump the player back to that exact moment.
5. Rate the film in the right panel and tag its scare types. The library sidebar ranks everything automatically — sort by overall score or any single category (scare, cinematography, sound, premise, acting…).

## Keyboard shortcuts

| Key | Action |
|---|---|
| `T` | Grab current timestamp into a note |
| `Space` | Play / pause |
| `←` / `→` | Skip back / forward 5 seconds |
| `Ctrl+Enter` (in note box) | Add note at current time |

## Rating categories

Scare Effectiveness · Cinematography · Sound Design · Premise/Concept · Acting · Pacing/Tension · Ending/Payoff · Rewatchability — each 0–10; the overall score is the average of the categories you've rated.

Scare-type tags: Jump Scare, Dread/Atmosphere, Psychological, Supernatural, Monster/Creature, Body Horror, Gore/Splatter, Slasher, Cosmic/Existential, Found Footage, Analog Horror, Folk Horror, Home Invasion, Tech Horror, Comedy Horror.

## Your data

Everything is saved automatically to your browser's local storage (per browser, per machine). Use **⬇ Export** to download your library as JSON for backup, and **⬆ Import** to merge it back in on another machine. Local video files can't be reopened automatically by browsers, so reload the file when you revisit a film — your notes and timestamps are all still there.
