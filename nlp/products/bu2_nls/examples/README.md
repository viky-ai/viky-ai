This directory contains a few examples fr testing some features.

Setup
=====

Copy directories packages and requests into the working directory:

```
cd ${OG_REPO_PATH}/products/bu2_nls/examples
cp -a ./packages $OG_REPO_PATH/ship/debug
cp -a ./requests $OG_REPO_PATH/ship/debug
cd $OG_REPO_PATH/ship/debug
```

Run examples :


List of examples
=====

Simple example:

```
ognlp -c packages/aller_de_a.json -i requests/r_i_want_to_go_from_new_york_to_barcelona.json
```

Simple recursive definition:

```
ognlp -c packages/go_town.json -i requests/r_go_town.json
```

Recursive definition of hotel features, thus defining an infinite list of hotel features:

```
ognlp -c packages/packages_want-hotel.json -c packages/packages_pg-building-feature.json -i requests/r_hotel_with_feature.json
ognlp -c packages/packages_pg-building-feature.json -i requests/r_with_features.json
```

Different tests for any:

```
ognlp -c packages/aller_de_a_plus_any.json -i requests/r_i_want_to_go_from_tokyo_to_paris.json
ognlp -c packages/aller_de_a_plus_any.json -i requests/r_i_want_to_go_from_new_york_to_barcelona.json
ognlp -c packages/aller_de_a_plus_any.json -i requests/r_i_want_to_go_from_new_york_to_paris.json
ognlp -c packages/packages_pg-building-feature-any.json -i requests/r_with_features_any.json
```

Summary of list of examples
=====

```
ognlp -c packages/aller_de_a.json -i requests/r_i_want_to_go_from_new_york_to_barcelona.json
ognlp -c packages/packages_want-hotel.json -c packages/packages_pg-building-feature.json -i requests/r_hotel_with_feature.json
ognlp -c packages/packages_pg-building-feature.json -i requests/r_with_features.json
ognlp -c packages/go_town.json -i requests/r_go_town.json
ognlp -c packages/aller_de_a_plus_any.json -i requests/r_i_want_to_go_from_tokyo_to_paris.json
ognlp -c packages/aller_de_a_plus_any.json -i requests/r_i_want_to_go_from_new_york_to_barcelona.json
ognlp -c packages/aller_de_a_plus_any.json -i requests/r_i_want_to_go_from_new_york_to_paris.json
ognlp -c packages/packages_pg-building-feature-any.json -i requests/r_with_features_any.json
```

