# CI Health & Green Checks (No Red X)

**Required status checks (Branch protection):** marque únicamente `ci` como obligatorio.
Opcionales: `clang-format` si desea enforcement de estilo.
Todos los demás workflows son no bloqueantes o se omiten en PRs desde forks.

Si aún ve ❌:
- Verifique que *Branch Protection* no exija otros checks.
- Revise que el job `build-test` en `ci` esté ejecutando en `ubuntu-latest` sin secretos.

## Required checks (Branch protection)
- Go to **Settings → Branches → Branch protection rules**.
- Enable **Require status checks to pass before merging**.
- Click **Search** and select **only** the check named **`ci`**.
- Keep all other checks **unchecked** (they are informative, non-blocking, or skipped on forks).

## Fork PRs
- By diseño, todos los jobs se **omiten** si el PR proviene de un fork (sin secretos).
- Si ves un rojo en un fork, revisa que la regla de Branch protection no esté pidiendo checks extra.

## Consejos
- Si quieres enforcement de estilo, marca `clang-format` como Required **solo si** estás seguro de que el repo está formateado y que todos los colaboradores podrán formatear localmente antes de subir.
- Para análisis pesados (CodeQL, security scans), usa `schedule:` diario o ejecuta manualmente con `workflow_dispatch`.
