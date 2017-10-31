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

# Build Nlp image

```
cd ${OG_REPO_PATH}
buildinit.pl
genmake.pl linux64 all
factory.pl -p bu2_nls fullclean
nettoyer.sh
docker build --target run_image -t docker-registry.pertimm.net/voqal.ai/platform/nlp .
```

# Run Nlp container
```
docker run -it --volume "$(pwd)/import:/nls/import" -p "9345:9345" -t docker-registry.pertimm.net/voqal.ai/platform/nlp
```
