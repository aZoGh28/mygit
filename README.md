# MyGit — une réimplémentation minimale de Git en C

> 🇫🇷 Version française ci-dessous — 🇬🇧 English version below.

## 🇫🇷 Français

**MyGit** est une réimplémentation pédagogique de quelques commandes de **Git**,
écrite en **C**, dans le but de comprendre en profondeur comment Git fonctionne
en interne (dossier `.git`, objets, hachage du contenu).

### Commandes réimplémentées
- **`init`** — crée l'arborescence `.git/` et `.git/objects/`.
- **`add`** — calcule et stocke les objets correspondant aux fichiers.
- Gestion des **objets Git** (`git_object.c` / `git_object.h`).
- Fonctions utilitaires (`utils.c` / `utils.h`) : allocation sûre, création de
  dossiers, gestion d'erreurs (`die`), etc.

### Structure
- `src/` — le code source C (`init.c`, `add.c`, `git_object.c`, `utils.c`, en-têtes).
- `compiled/` — binaires déjà compilés (`init`, `add`, `test`).
- `test.c` — petit programme de test.

### Compilation
```bash
gcc src/init.c   src/utils.c                     -o init
gcc src/add.c    src/utils.c src/git_object.c    -o add
```

### Note sur l'historique
Ce dépôt a été (re)versionné avec un historique Git propre. L'historique de
développement d'origine a été perdu : en travaillant sur la réimplémentation de
Git, le dossier `.git` local avait été modifié à la main et n'était plus lisible
par le Git officiel. Le code présent ici correspond à la dernière version du projet.

---

## 🇬🇧 English

**MyGit** is an educational reimplementation of a few **Git** commands, written in
**C**, aimed at deeply understanding how Git works internally (the `.git`
directory, objects, content hashing).

### Reimplemented commands
- **`init`** — creates the `.git/` and `.git/objects/` directory tree.
- **`add`** — computes and stores the objects for the given files.
- **Git object** handling (`git_object.c` / `git_object.h`).
- Utility helpers (`utils.c` / `utils.h`): safe allocation, directory creation,
  error handling (`die`), etc.

### Structure
- `src/` — the C source code (`init.c`, `add.c`, `git_object.c`, `utils.c`, headers).
- `compiled/` — pre-built binaries (`init`, `add`, `test`).
- `test.c` — a small test program.

### Build
```bash
gcc src/init.c   src/utils.c                     -o init
gcc src/add.c    src/utils.c src/git_object.c    -o add
```

### Note about history
This repository was re-versioned with a clean Git history. The original
development history was lost: while working on reimplementing Git, the local
`.git` directory had been hand-edited and could no longer be read by official
Git. The code here reflects the latest version of the project.
