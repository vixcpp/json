# =========================
# Vix.cpp — CLI: Release Makefile (safe & linear)
# =========================

VERSION      ?= v0.1.0
REMOTE        = origin
BRANCH_DEV    = dev
BRANCH_MAIN   = main

.PHONY: help status guard-clean guard-gh sync ffmerge merge-or-pr changelog commit push merge tag release test

help:
	@echo "Available commands:"
	@echo "  make status                 - Show divergence between $(REMOTE)/$(BRANCH_MAIN) and $(REMOTE)/$(BRANCH_DEV)"
	@echo "  make sync                   - Rebase $(BRANCH_DEV) on $(REMOTE)/$(BRANCH_MAIN) and align local $(BRANCH_MAIN)"
	@echo "  make ffmerge                - Fast-forward merge $(BRANCH_DEV) -> $(BRANCH_MAIN) and push"
	@echo "  make merge-or-pr            - FF merge if possible; otherwise open a PR (requires gh)"
	@echo "  make changelog              - Update CHANGELOG.md using script"
	@echo "  make commit                 - Commit local changes on $(BRANCH_DEV)"
	@echo "  make push                   - Push $(BRANCH_DEV)"
	@echo "  make tag VERSION=vX.Y.Z     - Create and push annotated tag"
	@echo "  make release VERSION=vX.Y.Z - changelog + commit + push + sync + ffmerge + tag"
	@echo "  make test                   - Run tests (ctest)"

status:
	@git fetch $(REMOTE) --tags
	@echo "== Divergence =="
	@echo "* Commits on $(BRANCH_DEV) not in $(BRANCH_MAIN):"
	@git log --oneline $(REMOTE)/$(BRANCH_MAIN)..$(REMOTE)/$(BRANCH_DEV) || true
	@echo
	@echo "* Commits on $(BRANCH_MAIN) not in $(BRANCH_DEV):"
	@git log --oneline $(REMOTE)/$(BRANCH_DEV)..$(REMOTE)/$(BRANCH_MAIN) || true
	@echo
	@echo "== Heads =="
	@git rev-parse --short $(REMOTE)/$(BRANCH_MAIN)
	@git rev-parse --short $(REMOTE)/$(BRANCH_DEV)

guard-clean:
	@if [ -n "$$(git status --porcelain)" ]; then \
		echo "❌ Working tree not clean. Commit or stash first."; \
		git status --porcelain; \
		exit 1; \
	fi

guard-gh:
	@command -v gh >/dev/null 2>&1 || { echo "❌ GitHub CLI (gh) not found. Install gh or use 'make ffmerge'."; exit 1; }

sync: ## rebase dev on origin/main; align local main with origin/main
	@git fetch $(REMOTE) --tags
	@git switch $(BRANCH_DEV)
	@git rebase $(REMOTE)/$(BRANCH_MAIN) || { echo "❌ Rebase failed. Resolve conflicts then 'git rebase --continue'."; exit 1; }
	@git push --force-with-lease $(REMOTE) $(BRANCH_DEV)
	@git switch $(BRANCH_MAIN)
	@git reset --hard $(REMOTE)/$(BRANCH_MAIN)

ffmerge: ## fast-forward only merge dev -> main
	@git switch $(BRANCH_MAIN)
	@git merge --ff-only $(BRANCH_DEV)
	@git push $(REMOTE) $(BRANCH_MAIN)

merge-or-pr: ## ff-only else open PR
	@git switch $(BRANCH_MAIN)
	@git merge --ff-only $(BRANCH_DEV) || ( \
		echo "ℹ️  FF impossible, creating PR…"; \
		$(MAKE) guard-gh; \
		gh pr create -B $(BRANCH_MAIN) -H $(BRANCH_DEV) -t "Merge $(BRANCH_DEV) into $(BRANCH_MAIN)" -b "Auto-PR from Makefile (FF not possible)"; \
	)

changelog:
	bash scripts/update_changelog.sh

commit:
	@git switch $(BRANCH_DEV)
	@if [ -n "$$(git status --porcelain)" ]; then \
		echo "📝 Committing changes…"; \
		git add .; \
		git commit -m "chore(release): prepare $(VERSION)"; \
	else \
		echo "✅ Nothing to commit."; \
	fi

push:
	@git push $(REMOTE) $(BRANCH_DEV)

tag:
	@if git rev-parse $(VERSION) >/dev/null 2>&1; then \
		echo "❌ Tag $(VERSION) already exists."; \
		exit 1; \
	else \
		echo "🏷️  Creating annotated tag $(VERSION)…"; \
		git tag -a $(VERSION) -m "Release $(VERSION)"; \
		git push $(REMOTE) $(VERSION); \
	fi

release: changelog commit push sync ffmerge tag
	@echo "✅ Release pipeline done: $(VERSION)"

test:
	@cd build && ctest --output-on-failure || true

