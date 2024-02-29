Just to learn, going be useful in another project.

its a pass that transform llvm binaryoperator instruction in functions

ex:
``` llvm
; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main(i32 noundef %argc, i8** noundef %argv) #0 {
entry:
  %mul = mul nsw i32 %argc, %argc
  ret i32 %mul
}
```
the %mul instruction will turn into a call to a function that calculates the mul.

in C:
``` c
int main(int argc, char **argv){
  int a = argc * argc;
  return argc;
}
```
will becomes:
``` c
int f(int a){
  return a*a;
}
int main(int argc, char **argv){
  int a = f(argc);
  return a;
}

```
