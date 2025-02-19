build_lib_example(
  NAME simple-sionna-example
  SOURCE_FILES simple-sionna-example.cc
  LIBRARIES_TO_LINK
    ${libsionna}
    ${libcore}
    ${liblte}
    ${libinternet}
    ${libnetwork}
    ${libmobility}
    ${libapplications}
)