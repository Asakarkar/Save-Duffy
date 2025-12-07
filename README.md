# Save Duffy 🐶✨  
*A retro-style SDL2 word-guessing game where you must save the dog by guessing the correct word!*

---

## 🎮 Overview  
**Save Duffy** is a pixel-art mini-game written in **C (C11)** using **SDL2**, **SDL2_image**, and **SDL2_ttf**.

You must guess the hidden word using the alphabet keys.  
Every wrong guess reduces a heart.  
If hearts reach zero… the dog dies ☠️🐶  
Guess correctly, and you *save* him ❤️

The game includes:

- Smooth dog animations (idle/blink, tail, run, death)
- Hint/context for each word
- Heart-based life system
- Clean retro UI panel
- Random word selection
- Pixel-art rendering with nearest-neighbor scaling

---

## 🐕 Game Preview  
*(Add your screenshots inside `assets/screenshots/` and update paths below)*

| Idle Animation | Tail Animation |
|----------------|----------------|
| ![Idle](assets/screenshots/idle.png) | ![Tail](assets/screenshots/tail.png) |

| Run Animation | Death Animation |
|---------------|-----------------|
| ![Run](assets/screenshots/run.png) | ![Death](assets/screenshots/death.png) |

---

## 🧩 Gameplay  
- Press **SPACE** to start  
- Press **A–Z** to guess letters  
- View **hint** from the right panel  
- Wrong guesses = lose hearts  
- Guess full word = **save the dog**  
- Zero hearts = **death animation**  
- Press SPACE to play again  

---

## 🛠️ Tech Stack  
| Component | Library |
|----------|----------|
| Rendering | SDL2 |
| Image loading | SDL2_image |
| Fonts & text | SDL2_ttf |
| Language | C (C11) |
| Build system | Makefile |

---

## 📂 Project Structure
Save-Duffy/ <br>
│ <br>
├── assets/ <br>
│ ├── sprites/ <br>
│ │ ├── dog_idle.png <br>
│ │ ├── dog_tail.png <br>
│ │ ├── dog_run.png <br>
│ │ └── dog_death.png <br>
│ ├── ui/ <br>
│ │ ├── heart_full.png <br>
│ │ └── heart_empty.png <br>
│ ├── font.ttf <br>
│ └── screenshots/ <br>
│ ├── idle.png <br>
│ ├── tail.png <br>
│ ├── run.png <br>
│ └── death.png <br>
│ <br>
├── src/ <br>
│ └── main.c <br>
│ <br>
├── Makefile <br>
├── README.md <br>
└── LICENSE <br>

## 🚀 Features
- 🌟 Retro 90’s pixel-animation aesthetic
- ✨ Automatic animation switching (idle → tail → run)
- 💓 Heart-based life UI
- 🧠 Word hints for accessibility
- 🎯 Keyboard-only gameplay
- 🔤 Wrapped text rendering with SDL2_ttf

## 📌 Future Improvements
- Sound effects
- Background music
- Difficulty levels
- More animations
- Larger word list
- High-score / streak tracking
