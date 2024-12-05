# Release Procedure

From the root directory:

<br/>

# Docs
- APIs
- Server config
- Tutorials
- Descriptions (i.e. `docs/docs/intro.md`)
- Install link in `docs/docs/install/package.md`

<br/>

# Tasks

<br/>

## Version
- Version in `core/NemesisCommon.h`

<br/>

## Package

```bash
cd package
./create.sh <version>
```

- Upload package to Azure

<br/>

## Docker

```bash
./docker_build.sh <version>
```
<br/>

## Docs

Build:

```bash
cd docs
npm run build
```

- Copy from `docs/build` to `docs-deploy` overwriting everything
- Commit and push docs-deploy

<br/>

# Final Checks
- Docs
  - Updated docs is [live](https://docs.nemesisdb.io/)
  - Download link works
- Package in Azure
- Download links work
