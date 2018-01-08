Pulling external dependencies
=================================

```
git submodule sync --recursive
git submodule update --init --recursive
```

Add new external dependencies
=================================

Clone it our gitlab : http://gitlab.pertimm.corp/viky.ai/dependencies

Sample adding `glib` use http url:

```
cd ${OG_REPO_PATH}/sources/external_dependencies
git submodule add "http://gitlab.pertimm.corp/viky.ai/dependencies/glib.git" glib

# choose tag to use
cd glib
git checkout 2.54.1
cd -

git add ${OG_REPO_PATH}/../.gitmodules glib
git commit -m "#15937 [added] Nls : glib (2.54.1) as external dependencies"
git push
```



Updating external dependencies
=================================

TODO
