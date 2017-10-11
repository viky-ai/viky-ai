Pulling external dependencies
=================================

```
git submodule sync --recursive
git submodule update --init --recursive
```

Add new external dependencies
=================================

Clone it our gitlab : http://gitlab.pertimm.corp/voqal.ai/dependencies

Sample adding `glib` :

```
cd ${OG_REPO_PATH}/sources/external_dependencies
git submodule add "git@gitlab.pertimm.corp:voqal.ai/dependencies/glib.git" glib

# choose tag to use
cd glib
git checkout v1.15
cd -

# Change absolute relative to relative one :
#  https://docs.gitlab.com/ce/ci/git_submodules.html#configuring-the-gitmodules-file
vim ${OG_REPO_PATH}/../.gitmodules

git add ${OG_REPO_PATH}/../.gitmodules glib
git commit -m "#15937 [added] Nls : glib (2.54.1) as external dependencies"
git push
```



Updating external dependencies
=================================

TODO
