function(CopyRuntimeDlls IN_FROM_TARGET IN_TO_TARGET)
  add_custom_command (TARGET ${IN_TO_TARGET} POST_BUILD
      COMMAND python ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/copy_s.py "$<TARGET_RUNTIME_DLLS:${IN_FROM_TARGET}>" "$<TARGET_FILE_DIR:${IN_TO_TARGET}>"
  )
endfunction()

function(LinkToExecutable IN_EXECUTABLE IN_TARGET)
  #SetOutputDir(${IN_TARGET} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
  target_link_libraries(${IN_EXECUTABLE} PUBLIC ${IN_TARGET})

  CopyRuntimeDlls(${IN_TARGET} ${IN_EXECUTABLE})
endfunction()

