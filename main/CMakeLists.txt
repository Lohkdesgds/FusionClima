idf_component_register(
    SRCS 
        "main.cpp"
        "as_client.cpp"
        "as_host.cpp"
        "shared/import/qrcodegen.cpp"
        "shared/buttons.cpp"
        "shared/display.cpp"
        "shared/import/DHT.cpp"
        "shared/wifictl.cpp"
    INCLUDE_DIRS ".")

component_compile_options(-std=gnu++17)