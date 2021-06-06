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
)

foreach(sample ${MATCHIT_SAMPLES})
    add_executable(${sample} ${sample}.cpp)
    target_compile_options(${sample} PRIVATE ${BASE_COMPILE_FLAGS})
    target_link_libraries(${sample} PRIVATE matchit)
    set_target_properties(${sample} PROPERTIES CXX_EXTENSIONS OFF)
endforeach()