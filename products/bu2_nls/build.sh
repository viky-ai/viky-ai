# bu2_internal
nettoyer.sh

buildinit.pl -u

genmake.pl linux64 all

# factory.pl -p bu2_nls fullclean

factory.pl -p bu2_nls redebug
factory.pl -s bu2_nls   debug

# Debug
ln -sfn `readlink -f $OG_REPO_PATH/ship/nls-* | head -n 1` $OG_REPO_PATH/ship/debug
cd $OG_REPO_PATH/ship/debug
