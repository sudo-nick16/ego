let a = [1, 2, 3, 4, 5]
let n = 5

let i = 0
let f = false
while (i+1 < n) {
    if (a[i] % 2 == 0 and a[i+1] % 2 == 0) {
        f = true
        println("found even pair")
    }
    i = i + 1
}
if (f == false) {
    println("no even pair found")
}
