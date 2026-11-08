# Add the Go binding test.
add_go_binding("" test_go_binding)

if (BUILD_GO_BINDINGS)
configure_file(go_binding_test.go.in ${CMAKE_CURRENT_BINARY_DIR}/go_binding_test.go COPYONLY)
add_test(NAME go_binding_test
    COMMAND ${GO_EXECUTABLE} test -v  ${CMAKE_CURRENT_BINARY_DIR}/go_binding_test.go
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/mlpack/bindings/go/src/github.com/milvi/mlpack-go/)
endif()
