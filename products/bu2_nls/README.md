après les checkouts :

export nproc = <nombre de proc de la machine -1>
export OG_APPS_PATH=`pwd`
export OG_REPO_PATH=${OG_APPS_PATH}/pse
export OG_TOOL_PATH=${OG_REPO_PATH}/tools
export PATH=${OG_TOOL_PATH}/bin:${PATH}
export MAKE_OPTS_PARALLEL=-j$(nproc)

buildinit.pl -u
genmake.pl linux64 all
factory.pl -p bu2_nls redebug
factory.pl -s bu2_nls   debug
cd ${OG_REPO_PATH}/ship/debug

Run
===
`./ogm_nls -d`

Tests
===
cd ${OG_REPO_PATH}/products/bu2_nls/tests
bundle install --path=cache/bundler --jobs $(nproc)
bundle exec rake test
