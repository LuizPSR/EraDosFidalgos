include(FetchContent)

FetchContent_Declare(
        glm_repo
        URL            https://github.com/g-truc/glm/archive/1.0.2.tar.gz
        URL_HASH       SHA256=19edf2e860297efab1c74950e6076bf4dad9de483826bc95e2e0f2c758a43f65
	EXCLUDE_FROM_ALL)

FetchContent_MakeAvailable(glm_repo)
