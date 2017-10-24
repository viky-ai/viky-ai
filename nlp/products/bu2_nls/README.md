After checkouts :


Setup
=====

Put this variables in your `~/.bashrc` :

```
export OG_APPS_PATH=`pwd`
export OG_REPO_PATH=${OG_APPS_PATH}/pse
export OG_TOOL_PATH=${OG_REPO_PATH}/tools
export PATH=${OG_TOOL_PATH}/bin:${PATH}
```

Clone external dependencies :

```
git submodule sync --recursive
git submodule update --init --recursive
```

If you need cleanup :

```
nettoyer.sh
buildinit.pl
genmake.pl linux64 all
factory.pl -p bu2_nls fullclean
```

if you use eclipse :

```
geneclipse.pl linux64 all
```


Build
=====

If you want to build with ASAN set `export OG_ASAN_ENABLE=1`, https://code.google.com/p/address-sanitizer/wiki/AddressSanitizer
Your need also to export possibly this variable:
export ASAN_OPTIONS=abort_on_error=1:detect_leaks=0:detect_stack_use_after_return=1:debug=0

```
buildinit.pl -u
genmake.pl linux64 all
factory.pl -p bu2_nls redebug
factory.pl -s bu2_nls   debug
ln -sfn `readlink -f $OG_REPO_PATH/ship/nls-* | head -n 1` $OG_REPO_PATH/ship/debug
cd $OG_REPO_PATH/ship/debug

```

Run
===

```
cd ${OG_REPO_PATH}/ship/debug
./ogm_nls
```

Tests
===

```
cd ${OG_REPO_PATH}/products/bu2_nls/tests
bundle install
bundle exec rake test
```

Docker
============

# Build Nls image

```
cd ${OG_REPO_PATH}
buildinit.pl
genmake.pl linux64 all
factory.pl -p bu2_nls fullclean
nettoyer.sh
docker build -t docker-registry.pertimm.corp:50001/voqal.ai/platform/nls .
```

# Run Nls container
```
docker run -ti -p 9345:9345 --name nls docker-registry.pertimm.corp:50001/voqal.ai/platform/nls
```
