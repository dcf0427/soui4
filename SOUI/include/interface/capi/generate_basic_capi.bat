@echo off
REM Basic C API Generator for SOUI Interfaces
REM This batch script creates basic C API macro files for common interfaces

echo Generating basic C API headers for SOUI interfaces...

REM Create SAdapter-capi.h
echo Creating SAdapter-capi.h...
(
echo #ifndef __SADAPTER_CAPI_H__
echo #define __SADAPTER_CAPI_H__
echo.
echo #include "../SAdapter-i.h"
echo.
echo #ifdef __cplusplus
echo extern "C" {
echo #endif
echo.
echo /* ILvAdapter C API Macros */
echo #define ILvAdapter_AddRef^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>AddRef^(This^^)^)
echo.
echo #define ILvAdapter_Release^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>Release^(This^^)^)
echo.
echo #define ILvAdapter_getCount^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>getCount^(This^^)^)
echo.
echo #define ILvAdapter_getView^(This, position, pItem, pContainer^) \
echo     ^^(^(This^)-^>lpVtbl-^>getView^(This, position, pItem, pContainer^^)^)
echo.
echo #define ILvAdapter_getItemDesc^(This, position^) \
echo     ^^(^(This^)-^>lpVtbl-^>getItemDesc^(This, position^^)^)
echo.
echo #define ILvAdapter_getItemDesiredSize^(This, position, pContainer, szParent^) \
echo     ^^(^(This^)-^>lpVtbl-^>getItemDesiredSize^(This, position, pContainer, szParent^^)^)
echo.
echo #define ILvAdapter_InitByTemplate^(This, pXmlTemplate^) \
echo     ^^(^(This^)-^>lpVtbl-^>InitByTemplate^(This, pXmlTemplate^^)^)
echo.
echo #ifdef __cplusplus
echo }
echo #endif
echo.
echo #endif /* __SADAPTER_CAPI_H__ */
) > SAdapter-capi.h

REM Create SAnimation-capi.h
echo Creating SAnimation-capi.h...
(
echo #ifndef __SANIMATION_CAPI_H__
echo #define __SANIMATION_CAPI_H__
echo.
echo #include "../SAnimation-i.h"
echo.
echo #ifdef __cplusplus
echo extern "C" {
echo #endif
echo.
echo /* IAnimation C API Macros */
echo #define IAnimation_AddRef^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>AddRef^(This^^)^)
echo.
echo #define IAnimation_Release^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>Release^(This^^)^)
echo.
echo #define IAnimation_copy^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>copy^(This^^)^)
echo.
echo #define IAnimation_start^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>start^(This^^)^)
echo.
echo #define IAnimation_pause^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>pause^(This^^)^)
echo.
echo #define IAnimation_resume^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>resume^(This^^)^)
echo.
echo #define IAnimation_stop^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>stop^(This^^)^)
echo.
echo #define IAnimation_isStarted^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>isStarted^(This^^)^)
echo.
echo #define IAnimation_getStartTime^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>getStartTime^(This^^)^)
echo.
echo #define IAnimation_getDuration^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>getDuration^(This^^)^)
echo.
echo #ifdef __cplusplus
echo }
echo #endif
echo.
echo #endif /* __SANIMATION_CAPI_H__ */
) > SAnimation-capi.h

REM Create SFactory-capi.h
echo Creating SFactory-capi.h...
(
echo #ifndef __SFACTORY_CAPI_H__
echo #define __SFACTORY_CAPI_H__
echo.
echo #include "../SFactory-i.h"
echo.
echo #ifdef __cplusplus
echo extern "C" {
echo #endif
echo.
echo /* IFactory C API Macros */
echo #define IFactory_AddRef^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>AddRef^(This^^)^)
echo.
echo #define IFactory_Release^(This^) \
echo     ^^(^(This^)-^>lpVtbl-^>Release^(This^^)^)
echo.
echo #define IFactory_CreateObject^(This, ppObj^) \
echo     ^^(^(This^)-^>lpVtbl-^>CreateObject^(This, ppObj^^)^)
echo.
echo #ifdef __cplusplus
echo }
echo #endif
echo.
echo #endif /* __SFACTORY_CAPI_H__ */
) > SFactory-capi.h

echo Basic C API headers generated successfully!
echo.
echo Generated files:
echo - SAdapter-capi.h
echo - SAnimation-capi.h  
echo - SFactory-capi.h
echo.
echo Note: These are basic templates. You may need to add more methods
echo based on the actual interface definitions.
