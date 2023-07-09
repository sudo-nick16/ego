func fact(n) {
    let ans = 1
    let f = 1
    while (f <= n) {
        ans = ans * f
        f = f + 1
    }
    return ans
}

println("factorial of 5 = ", fact(5))
