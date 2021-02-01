# xklee Install



### build minisat

```
cmake -DSTATICCOMPILE=ON -DCMAKE_INSTALL_PREFIX=/usr/ ../
```

### build stp

```
cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DENABLE_PYTHON_INTERFACE:BOOL=OFF ..
```