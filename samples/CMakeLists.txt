set(MATCHIT_SAMPLES
checkAndlogLarge
clip
contains
customAsPointer
customDs
detectTuplePattern
eval
factorial
gcd
getClassName
isLarge
isValid
optionalLift
relu
someNone
sumIs
variantAny
symmetric
quotientRemainder
Literal-pattern
Identifier-pattern
Wildcard-pattern
Rest-pattern
Range-pattern
Reference-pattern
Struct-pattern
Tuple-struct-pattern
Slice-pattern
Conditional-if-let-Expressions
While-conditional-let-Loops
Destructuring-Enums
Destructuring-Nested-Structs-and-Enums
Ignoring-Parts-of-a-Value-with-a-Nested-_
Extra-Conditionals-with-Match-Guards
at-Bindings
)

foreach(sample ${MATCHIT_SAMPLES})
    add_executable(${sample} ${sample}.cpp)
    target_compile_options(${sample} PRIVATE ${BASE_COMPILE_FLAGS})
    target_link_libraries(${sample} PRIVATE matchit)
    set_target_properties(${sample} PROPERTIES CXX_EXTENSIONS OFF)
    add_test(${sample} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${sample})
endforeach()