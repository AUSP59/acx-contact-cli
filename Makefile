.PHONY: build test pack docs fuzz format lint bench
build:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release
test:
	ctest --test-dir build --output-on-failure
pack:
	cpack -C Release -B dist
docs:
	pip install -q mkdocs-material && mkdocs build -d site
fuzz:
	bash scripts/fuzz.sh
format:
	bash scripts/format.sh
lint:
	bash scripts/lint.sh
bench:
	python3 scripts/bench_ci.py


dist: build
	cpack -C Release -B dist
	bash scripts/checksums.sh || true
	bash scripts/verify_release.sh || true


all-checks:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
	ctest --test-dir build --output-on-failure || true
	bash scripts/verify_release.sh || true
	bash scripts/checksums.sh || true
	python3 scripts/sbom_license_policy.py SBOM.cyclonedx.json || true
