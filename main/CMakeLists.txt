idf_component_register(SRCS "main.cpp" "as_client.cpp" "as_host.cpp" "shared/buttons.cpp" "shared/display.cpp" "shared/DHT.cpp"
                    INCLUDE_DIRS ".")

component_compile_options(-std=gnu++17)