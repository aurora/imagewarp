add_rules("mode.debug", "mode.release")
add_requires("CONAN::nlohmann_json/3.10.5", { alias = "json" })
add_requires("opencv 4.2.0", { alias = "opencv", configs = { shared = false, gtk = false } })

target("imagewarp")
    set_kind("binary")
    add_files("src/*.cpp")
    add_packages("opencv", "json")
