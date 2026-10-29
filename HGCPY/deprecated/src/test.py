import lib.pyground as pyground


def add(arg1, arg2):
    result = arg1 + arg2
    print(result)


pyground.set_function(add)
pyground.call_function(1, 1)
