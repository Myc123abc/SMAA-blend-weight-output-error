This project is a minimize code to solve SMAA blend weight output error leading SMAA output image uncomplete Anti-Aliasing.

External libraries as follow:
* https://github.com/libsdl-org/SDL release-3.2.12
* https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git v3.2.1
* https://github.com/iryoku/smaa.git master

Test use vulkan version 1.4.309

Windows 10.0.19045
64bit
Clang 18.1.8

External build warnning:
```
C:/Users/myc/Desktop/SMAA-blend-weight-output-error/build/_deps/sdl3-src/src/stdlib/SDL_string.c:1157:12: warning: 'itoa' is deprecated: The POSIX name for this item is deprecated. Instead, use 
the ISO C and C++ conformant name: _itoa. See online help for details. [-Wdeprecated-declarations]
 1157 |     return itoa(value, string, radix);
      |            ^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt\stdlib.h:1315:5: note: 'itoa' has been explicitly marked deprecated here
 1315 |     _CRT_NONSTDC_DEPRECATE(_itoa) _CRT_INSECURE_DEPRECATE(_itoa_s)
      |     ^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt\corecrt.h:428:50: note: expanded from macro '_CRT_NONSTDC_DEPRECATE'
  428 |         #define _CRT_NONSTDC_DEPRECATE(_NewName) _CRT_DEPRECATE_TEXT(             \
      |                                                  ^
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\include\vcruntime.h:358:47: note: expanded from macro '_CRT_DEPRECATE_TEXT'
  358 | #define _CRT_DEPRECATE_TEXT(_Text) __declspec(deprecated(_Text))
      |                                               ^
C:/Users/myc/Desktop/SMAA-blend-weight-output-error/build/_deps/sdl3-src/src/stdlib/SDL_string.c:1175:12: warning: '_ltoa' is deprecated: This function or variable may be unsafe. Consider using 
_ltoa_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details. [-Wdeprecated-declarations]
 1175 |     return _ltoa(value, string, radix);
      |            ^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt\stdlib.h:664:1: note: '_ltoa' has been explicitly marked deprecated here
  664 | __DEFINE_CPP_OVERLOAD_STANDARD_FUNC_1_1(
      | ^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt\corecrt.h:847:5: note: expanded from macro '__DEFINE_CPP_OVERLOAD_STANDARD_FUNC_1_1'
  847 |     __DEFINE_CPP_OVERLOAD_STANDARD_FUNC_1_1_EX(_ReturnType, _ReturnPolicy, _DeclSpec, _FuncName, _FuncName##_s, _HType1, _HArg1, _SalAttributeDst, _DstType, _Dst, _TType1, _TArg1)       
      |     ^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt\corecrt.h:1906:17: note: expanded from macro '__DEFINE_CPP_OVERLOAD_STANDARD_FUNC_1_1_EX'
 1906 |                 _CRT_INSECURE_DEPRECATE(_SecureFuncName) _DeclSpec _ReturnType __cdecl _FuncName(_HType1 _HArg1, _SalAttributeDst _DstType *_Dst, _TType1 _TArg1);
      |                 ^
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\include\vcruntime.h:368:55: note: expanded from macro '_CRT_INSECURE_DEPRECATE'
  368 |         #define _CRT_INSECURE_DEPRECATE(_Replacement) _CRT_DEPRECATE_TEXT(    \
      |                                                       ^
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\include\vcruntime.h:358:47: note: expanded from macro '_CRT_DEPRECATE_TEXT'
  358 | #define _CRT_DEPRECATE_TEXT(_Text) __declspec(deprecated(_Text))
      |                                               ^
C:/Users/myc/Desktop/SMAA-blend-weight-output-error/build/_deps/sdl3-src/src/stdlib/SDL_string.c:1193:12: warning: '_ultoa' is deprecated: This function or variable may be unsafe. Consider using _ultoa_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details. [-Wdeprecated-declarations]
 1193 |     return _ultoa(value, string, radix);
      |            ^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt\stdlib.h:687:1: note: '_ultoa' has been explicitly marked deprecated here
  687 | __DEFINE_CPP_OVERLOAD_STANDARD_FUNC_1_1(
      | ^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt\corecrt.h:847:5: note: expanded from macro '__DEFINE_CPP_OVERLOAD_STANDARD_FUNC_1_1'
  847 |     __DEFINE_CPP_OVERLOAD_STANDARD_FUNC_1_1_EX(_ReturnType, _ReturnPolicy, _DeclSpec, _FuncName, _FuncName##_s, _HType1, _HArg1, _SalAttributeDst, _DstType, _Dst, _TType1, _TArg1)       
      |     ^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt\corecrt.h:1906:17: note: expanded from macro '__DEFINE_CPP_OVERLOAD_STANDARD_FUNC_1_1_EX'
 1906 |                 _CRT_INSECURE_DEPRECATE(_SecureFuncName) _DeclSpec _ReturnType __cdecl _FuncName(_HType1 _HArg1, _SalAttributeDst _DstType *_Dst, _TType1 _TArg1);
      |                 ^
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\include\vcruntime.h:368:55: note: expanded from macro '_CRT_INSECURE_DEPRECATE'
  368 |         #define _CRT_INSECURE_DEPRECATE(_Replacement) _CRT_DEPRECATE_TEXT(    \
      |                                                       ^
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\include\vcruntime.h:358:47: note: expanded from macro '_CRT_DEPRECATE_TEXT'
  358 | #define _CRT_DEPRECATE_TEXT(_Text) __declspec(deprecated(_Text))
      |                                               ^
C:/Users/myc/Desktop/SMAA-blend-weight-output-error/build/_deps/sdl3-src/src/stdlib/SDL_string.c:1217:12: warning: '_i64toa' is deprecated: This function or variable may be unsafe. Consider using _i64toa_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details. [-Wdeprecated-declarations]
 1217 |     return _i64toa(value, string, radix);
      |            ^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt\stdlib.h:704:1: note: '_i64toa' has been explicitly marked deprecated here
  704 | _CRT_INSECURE_DEPRECATE(_i64toa_s)
      | ^
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\include\vcruntime.h:368:55: note: expanded from macro '_CRT_INSECURE_DEPRECATE'
  368 |         #define _CRT_INSECURE_DEPRECATE(_Replacement) _CRT_DEPRECATE_TEXT(    \
      |                                                       ^
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\include\vcruntime.h:358:47: note: expanded from macro '_CRT_DEPRECATE_TEXT'
  358 | #define _CRT_DEPRECATE_TEXT(_Text) __declspec(deprecated(_Text))
      |                                               ^
C:/Users/myc/Desktop/SMAA-blend-weight-output-error/build/_deps/sdl3-src/src/stdlib/SDL_string.c:1235:12: warning: '_ui64toa' is deprecated: This function or variable may be unsafe. Consider using _ui64toa_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details. [-Wdeprecated-declarations]
 1235 |     return _ui64toa(value, string, radix);
      |            ^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt\stdlib.h:720:1: note: '_ui64toa' has been explicitly marked deprecated here
  720 | _CRT_INSECURE_DEPRECATE(_ui64toa_s)
      | ^
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\include\vcruntime.h:368:55: note: expanded from macro '_CRT_INSECURE_DEPRECATE'
  368 |         #define _CRT_INSECURE_DEPRECATE(_Replacement) _CRT_DEPRECATE_TEXT(    \
      |                                                       ^
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\include\vcruntime.h:358:47: note: expanded from macro '_CRT_DEPRECATE_TEXT'
  358 | #define _CRT_DEPRECATE_TEXT(_Text) __declspec(deprecated(_Text))
      |                                               ^
5 warnings generated.
```