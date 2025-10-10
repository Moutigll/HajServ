# Contributing to HajServ

Thank you for considering contributing!
This guide explains the workflow, code style, testing, PR process, and communication guidelines for the project. Please read carefully before opening issues or PRs.

---

## Branching & Workflow

- **Never branch from `main`**. Always branch from `dev` or `v3` (latest stable development branch).
- Branch naming convention:
	- `feature/<short-description>`
	- `bugfix/<short-description>`
	- `refactor/<short-description>`
- Pull Requests (PRs) must target the **branch they were created from** (`dev` or `v3`).
- PRs should link to the relevant **milestone** and **issue** (if applicable).

---

## Coding Standards

### C Code
- Function and variable names: **camelCase**  
```c
void handleRequest(wsRequest *req, wsResponse *res);
```
- File names: UpperCamelCase
- Use real tabs for indentation.
- Try to keep lines under 80 characters, but readability takes priority.
- Doxygen-style documentation is required for all public functions:
```c
/**
 * @brief Handles an HTTP request and prepares a response.
 * @param req Pointer to the request object.
 * @param res Pointer to the response object.
 */
void handleRequest(wsRequest* req, wsResponse* res);
```
- Clean, modular, and readable code is mandatory. Avoid overly complex one-liners or hidden behavior.
- Memory management: always free allocated memory, avoid leaks.

---

## Pull Requests (PRs)

- PR title should be descriptive, e.g.:
```pgsql
[FEATURE] Add HTTP/2 stream parser
[BUG] Fix memory leak in WebSocket handler
```
+ Include a summary of changes and motivation.
- Link the PR to a milestone and, if relevant, an issue.
- Use the PR template for all submissions.
- Reviewers may adjust labels or provide suggestions before merging.

---

## Issues

- Always use the provided issue templates (`Bug Report`, `Feature Request`).
- Do not create blank issues.
- Link PRs to the issues they resolve.
- Provide steps to reproduce bugs or detailed descriptions for feature requests.

---

## Communication Guidelines

- Be respectful and concise.
- Avoid harsh language or public blaming.
- Discuss large changes or architecture decisions in an issue first before implementing.
- Respond to comments on PRs within a reasonable timeframe.

Thank you for contributing to HajServ! Your work makes this project better and stronger.
