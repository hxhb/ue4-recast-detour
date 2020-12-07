### What is this?

This is recast navigation detour tool.It can verify the Recast Navigation data exported from the UE.

you can use **[ue4-export-nav-data](https://github.com/hxhb/ue4-export-nav-data)** plugin export recast navigation data from the UE4.

## How do use?

```bash
$ ue4-detour.exe ThirdPersonExampleMap.bin
Usage:
        ue4-detour.exe dtNavMesh.bin Loc.X Loc.Y,loc.Z Extren.X Extern.Y Extren.Z
PS:{Extern.X Extern.Y Extern.Z} can be ignored,default is {10.f 10.f 10.f}
For Example:
        ue4-detour.exe dtNavMesh.bin -770.003 -593.709 130.267 10.0 10.0 10.0
        
$ ue4-detour.exe ThirdPersonExampleMap.bin -770.003 -593.709 130.267
Deserialize ThirdPersonExampleMap.bin as dtNavMesh successfuly!
InPoint: X=-770.002991  Y=-593.708984   Z=130.266998
Extern: X=10.000000     Y=10.000000     Z=10.000000
The Location is valid navigation position.
```

## Compile

```bash
g++ -std=c++11 -o test.exe *.cpp *.h Detour/*.cpp Detour/*.h -D NDEBUG
```
使用VS可以新建一个空的解决方案添加仓库中的代码。
