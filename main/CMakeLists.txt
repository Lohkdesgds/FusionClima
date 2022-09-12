idf_component_register(
    SRCS 
        "main.cpp"
        "as_client.cpp"
        "as_host.cpp"
        "shared/import/qrcodegen.cpp"
        "shared/import/dns_server.cpp"
        "shared/import/LoRa.cpp"
        "shared/buttons.cpp"
        "shared/display.cpp"
        "shared/import/DHT.cpp"
        "shared/wifictl.cpp"
        "shared/cpuctl.cpp"
    INCLUDE_DIRS "."
    EMBED_FILES root.html)

component_compile_options(-std=gnu++17)