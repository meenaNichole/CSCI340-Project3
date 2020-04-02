/*References: 
	https://www.golang-book.com/books/intro/10
	https://yourbasic.org/golang/read-file-line-by-line/

*/

package main


import ("fmt"
	"os"
	"bufio"
)



func main(){
	args := os.Args[1:]

	fmt.Println(args)

	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan(){
		fmt.Println(scanner.Text())		
	}
}
