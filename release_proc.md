# Release Procedure

From the root directory:

<br/>

# Docs
- APIs updated
- Tutorials
- Descriptions (i.e. `docs/docs/intro.md`)
- Install link in `docs/docs/install/package.md`

<br/>

# Tasks

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

After build:
-  `docker push nemesisdb/nemesisdb:<version>`
-  `docker tag nemesisdb/nemesisdb:<version> nemesisdb/nemesisdb:latest`
-  `docker push nemesisdb/nemesisdb:latest`


## Docs

Build:

```bash
cd docs
npm run build
```

- Copy from `docs/build` to `docs-deploy` overwriting everything
- Commit and push docs-deploy

<br/>

## Website
- Update download button link

<br/>

# Final Checks
- Docs
  - Updated docs is [live](https://docs.nemesisdb.io/)
  - Download link works
- Package in Azure
- Download links work
