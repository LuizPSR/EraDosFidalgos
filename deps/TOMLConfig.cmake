include(FetchContent)

FetchContent_Declare(
        tomlplusplus
        URL            https://github.com/marzer/tomlplusplus/archive/v3.4.0.tar.gz
        URL_HASH       SHA256=8517f65938a4faae9ccf8ebb36631a38c1cadfb5efa85d9a72e15b9e97d25155
	EXCLUDE_FROM_ALL)

FetchContent_MakeAvailable(tomlplusplus)
