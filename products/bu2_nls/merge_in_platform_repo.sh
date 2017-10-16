# Fork pertimm_apps
git clone git@gitlab.pertimm.corp:voqal.ai/alpha/pertimm_apps.git pertimm_apps_forked
cd  pertimm_apps_forked

git st

# remove origin to avoid mistakes
git remote remove origin


# Include list get from after nls build
# cd $OG_REPO_PATH
# echo "" > used_include_raw.txt
# for file in $(find sources -name *.d); do
#   cat $file >> used_include_raw.txt
# done
# cat used_include_raw.txt | \
#   grep "/sources/include/" | \
#   grep -v "obj/debug/"  | \
#   grep -v "/usr/lib/" | \
#   grep -v "/sources/include/glib-2.0" | \
#   grep -v "/sources/include/jansson" | \
#   grep -v "/sources/include/uriparser" | \
#   sed 's@ \\@@g' | sed 's@ ogm_.*.h @@g' | \
#   sed 's@ /data/git/voqal.ai/pertimm_apps/@@g'  | \
#   sed 's@/data/git/voqal.ai/pertimm_apps/@@g'  | \
#   sort -u >  used_include.txt
# rm -f sed_include_raw.txt


export TO_KEEP_LIST="

  pse/Dockerfile
  pse/.dockerignore
  pse/.gitignore

  pse/products/bu2_nls
  pse/products/cmanifest.dtd
  pse/products/pmanifest.dtd
  pse/products/rmanifest.dtd
  pse/resources/manifest.xml

  pse/tools/bin/nettoyer.sh
  pse/tools/bin/ogutil.pm
  pse/tools/bin/buildinit.pl
  pse/tools/bin/genmake.pl
  pse/tools/bin/geneclipse.pl
  pse/tools/bin/factory.pl
  pse/tools/bin/lhp.sh
  pse/tools/makefile/linux
  pse/tools/makefile/makefile.dtd
  pse/tools/template/eclipse
  pse/tools/release/buildReport2html.xsl
  pse/tools/bin/valgrind_diff_clean.sh

  pse/sources/external_dependencies

  pse/sources/makefile.defs.linux
  pse/sources/manifest.xml

  pse/sources/include/logaddr.h
  pse/sources/include/logauta.h
  pse/sources/include/logaut.h
  pse/sources/include/logchset.h
  pse/sources/include/logfix.h
  pse/sources/include/loggen.h
  pse/sources/include/logghbn.h
  pse/sources/include/logheap.h
  pse/sources/include/loghttp.h
  pse/sources/include/logis3166.h
  pse/sources/include/logis639_3166.h
  pse/sources/include/logis639.h
  pse/sources/include/loglang.h
  pse/sources/include/logmess.h
  pse/sources/include/logmlog.h
  pse/sources/include/logmsg.h
  pse/sources/include/lognlp.h
  pse/sources/include/lognls.h
  pse/sources/include/logocc.h
  pse/sources/include/logpath.h
  pse/sources/include/logsig.h
  pse/sources/include/logsock.h
  pse/sources/include/logstat.h
  pse/sources/include/logsysi.h
  pse/sources/include/logtag.h
  pse/sources/include/logthr.h
  pse/sources/include/logtld.h
  pse/sources/include/loguci.h
  pse/sources/include/loguni.h
  pse/sources/include/logxpt.h
  pse/sources/include/logzlib.h
  pse/sources/include/lpcerr.h
  pse/sources/include/lpcgentype.h
  pse/sources/include/lpclat.h
  pse/sources/include/lpclog.h
  pse/sources/include/lpcmcr.h
  pse/sources/include/lpcosys.h
  pse/sources/include/lpcounx.h
  pse/sources/include/lpctime.h

  pse/sources/ogm_gen/lib
  pse/sources/ogm_aut
  pse/sources/ogm_xpt/lib
  pse/sources/ogm_thr/lib
  pse/sources/ogm_sig/lib
  pse/sources/ogm_auta
  pse/sources/ogm_uni/lib
  pse/sources/ogm_msg/lib
  pse/sources/ogm_sysi
  pse/sources/ogm_heap
  pse/sources/ogm_sock/lib
  pse/sources/ogm_zlib/lib
  pse/sources/ogm_ghbn/lib
  pse/sources/ogm_addr/lib
  pse/sources/ogm_http/lib
  pse/sources/ogm_uci

  pse/sources/ogm_nlp
  pse/sources/ogm_nls
"

export TO_KEEP=`echo "${TO_KEEP_LIST}" | tr '\n' ' '`

# Clean repo
git tag | xargs git tag -d
rm -rf .git/refs/original/
git reflog expire --expire=now --all
git gc --prune=now
git gc --aggressive --prune=now
git repack -A -d
du -h .git/objects/pack/*

# list directory to keep
git filter-branch -f --tag-name-filter 'cat' --index-filter 'git rm -f --cached -qr --ignore-unmatch -- . && git reset -q $GIT_COMMIT -- ${TO_KEEP}' --prune-empty -- --all

# Clean repo
rm -rf .git/refs/original/
git reflog expire --expire=now --all
git gc --prune=now
git gc --aggressive --prune=now
git repack -A -d
du -h .git/objects/pack/*

# Flatten dir
git subtree split --prefix=pse -b cleaned


#################
# voqal_platform
#################
cd ..
rm -rf platform_forked
git clone git@gitlab.pertimm.corp:bruzand/platform.git platform_forked
cd  platform_forked

git co -b 15937_merge_nls_repo
git push --set-upstream origin 15937_merge_nls_repo

git remote  add pertimm_apps ../pertimm_apps_forked
git subtree add --prefix=nls pertimm_apps cleaned


# Restore submodules
cat >> .gitmodules <<EOL
[submodule "uriparser"]
  path = nls/sources/external_dependencies/uriparser
  url = http://gitlab.pertimm.corp/voqal.ai/dependencies/uriparser.git
  branch = master
[submodule "jansson"]
  path = nls/sources/external_dependencies/jansson
  url = http://gitlab.pertimm.corp/voqal.ai/dependencies/jansson.git
  branch = master
[submodule "glib"]
  path = nls/sources/external_dependencies/glib
  url = http://gitlab.pertimm.corp/voqal.ai/dependencies/glib.git
  branch = 2.54.1
EOL

git submodule sync
git submodule update --init nls/sources/external_dependencies/uriparser
git submodule update --init nls/sources/external_dependencies/jansson
git submodule update --init nls/sources/external_dependencies/glib

git add .gitmodules

git add nls/sources/external_dependencies/uriparser
git add nls/sources/external_dependencies/jansson
git add nls/sources/external_dependencies/glib

git commit -m "#15937 [added] Nls external dependencies : glib, uriparser, jansson"


# manualy merge nls/products/bu2_nls/gitlab-ci.yml in .gitlab-ci.yml
subl nls/products/bu2_nls/gitlab-ci.yml .gitlab-ci.yml

# remove "pse" everywhere (README.md, dockerfile, ..)
