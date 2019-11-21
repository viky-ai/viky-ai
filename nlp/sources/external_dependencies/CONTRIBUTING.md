Pulling external dependencies
=================================

```
git submodule sync --recursive
git submodule update --init --recursive
```

Add new/updating an external dependency
=================================

Sample adding `glib` use http url:

```
cd ${OG_REPO_PATH}/sources/external_dependencies
git submodule add "https://github.com/GNOME/glib.git" glib

# choose tag to use
cd glib
git checkout 2.62.2
cd -

git add ${OG_REPO_PATH}/../.gitmodules glib
git commit -m "[added] Nls : glib as external dependencies"
git push
```