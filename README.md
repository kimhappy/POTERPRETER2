# POTERPRETER2

[POTERPRETER](https://github.com/BABTUL/POTERPRETER)의 새 버전으로, 많은 버그가 수정되었고, 캡쳐 기능 강화를 위해 문법도 수정되었습니다. 코드 전격 교체!

현재 캡쳐시의 성능을 개선한 POTERPRETER 2.1과 PVM(POTERPRETER VIRTUAL MACHINE)을 사용한 POTERPRETER 3.0 작업이 동시에 진행되고 있습니다.

***

## 문법

‘뽀식이네 감자탕’은 실수와 함수, 두 개의 자료형을 제공합니다. `print_num`을 사용해 실수를 출력할 수 있고, `print_char`를 사용해 실수를 정수로 캐스팅한 후 그 값에 해당하는 아스키 코드를 출력합니다. 실수와 정수의 캐스팅 규칙은 구현체의 언어인 C++에 의존합니다. 함수를 호출할 때 괄호와 인자 사이에는 공백이 있으면 안 됩니다.

코드:
```
print_num(10)
print_char(65)
```

출력:
```
10
A
```

***

`//`로 주석을 만듭니다. 코드와 주석 사이에는 공백이 없습니다.

코드:
```
print_num(10)// 주석
//print_num(20)
```

출력:
```
10
```

***

백쿼트를 사용해 변수를 만들 수 있습니다. 등호를 사용해 반드시 초기화해야 합니다. 변수명과 등호, 값 사이에는 각각 한 칸씩의 공백을 둡니다. 변수에는 실수와 함수 모두 들어갈 수 있습니다.

코드:
```
`x = 10
`y = 40
print_num(x)
print_num(y)
x = 30
print_num(x)
print_num(`z = 20)
```

출력:
```
10.000000
40.000000
30.000000
20.000000
```

***

연산자를 대신할 몇 가지 기본 함수를 제공합니다. `not`, `add`, `sub`, `mul`, `div`, `pow`, `greater`, `less`, `greater_equal`, `less_equal`, `and`, `or`, `same`, `diff`, `if`입니다. 이 중 `and`, `or`, `if`에 대해서는 지연평가가 적용됩니다. 두 개 이상의 인자는 `,`로 구분하되 콤마 뒤에는 한 칸의 공백을 둡니다.

`0`은 거짓이고, 그 이외의 수는 참입니다. 기본 함수이 참값에 대해 반환하는 값은 `1`입니다. `print_num`과 `print_char`는 `1`을 반환합니다. 수학 계산을 위해 `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`, `log`(밑이 자연상수), `ceil`, `floor`, `round`를 제공합니다.

코드:
```
if(1, print_num(10), print_num(20))
print_num(mul(add(10, 20), sub(35, 30)))
print_num(if(less(10, 20), 100, 200))
```

출력:
```
10
15
100
```

***

`#def`와 `end`를 사용해 함수를 정의할 수 있습니다. 마지막 줄의 값이 반환됩니다. 단, 대입문은 반환값이 없습니다.

함수 내부에서 함수를 정의할 수 있습니다.

`NYA`는 함수 자기 자신을 가리킵니다. 이는 함수명에 얽매이지 않는, 일반화된 프로그래밍을 도와줍니다.

변수의 범위가 기존의 언어들과 다소 차이가 있습니다. 콜 스택을 거꾸로 올라가며 먼저 만나는 것을 기준으로 삼습니다. 현재 함수에서 선언된 것이 아니라도 상관없습니다. 또, 함수 내에서 사용되는 변수와 함수는 실행 시점에서만 유효하면 됩니다. 즉, 지연평가가 적용됩니다.

코드:
```
//함수 선언, 함수 내 함수 선언, 자기 자신을 가리키는 NYA
#def fibo(n)
  #def fibo_impl(n, a, b)
    if(same(n, 1), b, NYA(sub(n, 1), b, add(a, b)))
  end
  fibo_impl(n, 0, 1)
end

//고차함수
#def runner(func, para)
  func(para)
end

//함수 실행
runner(print_num, fibo(10))

//스코프 테스트
#def adefer()
  `a = 10
  bdefer()
end

#def bdefer()
  `b = 20
  cdefer()
end

#def cdefer()
  `c = 30
  adefer2()
end

#def adefer2()
  `a = 40
  printer()
end

#def printer()
  print_num(a)
  print_char(10)
  print_num(b)
  print_char(10)
  print_num(c)
end

adefer()
```

출력:
```
55.000000
40.000000
20.000000
30.000000
```

***

함수 선언시 `[`과 `]`로 감싼 부분은 캡쳐 예정인 부분입니다. `!` 연산자를 사용해 함수가 해당 부분들을 현재의 맥락에 맞게 캡쳐할 수 있도록 합니다. 캡쳐할 부분이 있으나 캡쳐하지 않은 채로 호출할 수 없습니다.

`lambda` 키워드를 사용해 람다 함수를 만듭니다. `,`로 줄을 구분할 수 있고, 람다에서도 캡쳐 기능을 사용할 수 있습니다.

코드:
```
//캡쳐를 사용한 바인딩
#def binder(func, para1)
  #def ret(para2)
    [func]([para1], para2)
  end
  !ret
end

//람다 함수
`f = binder(lambda(x1, x2)(add(x1, x2)), 10)
print_num(f(20))

`a = 10
`b = 20

//현재의 맥락에서 func가 정의되어 있지 않아도 됩니다.
`haha = lambda()([func(a, b)], print_char(10), func(a, b))
//여러줄 람다 
`func = lambda(x1, x2)(print_num(x1), print_char(10), print_num(x2))
//hoho에 현재의 맥락을 캡쳐한 haha가 들어갑니다.
`hoho = !haha

a = 30
b = 40
`func = lambda(x1, x2)(print_num(x2), print_char(10), print_num(x1))

hoho()
```

출력:
```
30.000000
10.000000
20.000000
40.000000
30.000000
```

## 예제

뽀식이네 감자탕은 컨테이너 자료구조를 제공하지 않습니다. 하지만, 클로저를 사용하면 이를 충분히 구현할 수 있습니다.

코드:
```
//배열 구현
`_size = -1

#def Arr(index)
  0
end

#def assign(a, i, v)
  `ret = lambda(index)(if(same(index, [i]), [v], [a](index)))
  !ret
end

//자주 캡쳐되는 것들은 미리 캡쳐해서 변수에 저장해둬야 효과적
#def insert(a, i, v)
  `ret = lambda(index)([`ca = a], [`ci = i], if(same(index, _size), add(ca(_size), 1), if(less(index, ci), ca(index), if(same(index, ci), [v], ca(sub(index, 1))))))    
  !ret
end

#def erase(a, i)
  `ret = lambda(index)([`ca = a], if(same(index, _size), sub(ca(_size), 1), if(less(index, [i]), ca(index), ca(add(index, 1)))))
  !ret
end

#def push_front(a, v)
  `ret = lambda(index)([`ca = a], if(same(index, _size), add(ca(_size), 1), if(same(index, 0), [v], ca(sub(index, 1)))))
  !ret
end

#def push_back(a, v)
  `ret = lambda(index)([`ca = a], if(same(index, _size), add(ca(_size), 1), if(same(index, ca(_size)), [v], ca(index))))
  !ret
end

#def pop_back(a)
  `ret = lambda(index)([`ca = a], if(same(index, _size), sub(ca(_size), 1), ca(index)))
  !ret
end

#def pop_front(a)
  `ret = lambda(index)([`ca = a]if(same(index, _size), sub(ca(_size), 1), ca(add(index, 1))))
  !ret
end

#def for_each(a, func)
  lambda(index)(if(less(index, a(_size)), add(func(a(index)), NYA(add(index, 1))), 0))(0)
end

//사용
`haha = Arr

haha = push_back(haha, 10)
haha = push_back(haha, 20)
haha = push_back(haha, 30)
haha = push_back(haha, 40)

for_each(haha, lambda(x)(print_num(x), print_char(10)))
```

출력:
```
10.000000
20.000000
30.000000
40.000000
```
