include(FetchContent)

FetchContent_Declare(
        SDL3
        URL      https://github.com/libsdl-org/SDL/archive/52a4366e544fbaf6571d914361dbac323467206a.tar.gz
        URL_HASH SHA256=18cd8e7c6d383f42e0737fe56bc8a8d6fda4b74daefd6af8813c1edf90baf638
	EXCLUDE_FROM_ALL)

FetchContent_MakeAvailable(SDL3)
