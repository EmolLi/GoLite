# GoLite

Go (often referred to as Golang) is a programming language created at Google in 2009 by Robert Griesemer, Rob Pike, and Ken Thompson. Go is a statically typed language in the tradition of C, with memory safety, garbage collection, structural typing, and CSP-style concurrent programming features added.

GoLite is a subset of Go, that contains interesting features including variable, function, type declaration, control structures like if-else statement, switch statement and a bunch of other basic Go features. 

The purpose of this project is to build a compiler for GoLite with target language of server side JavaScript (Node.js), to gain better understanding of modern compiler development via hands-on experience.

For more detail about the project, please check out the [report of the project](https://github.com/EmolLi/GoLite/blob/master/doc/finalReport.pdf).

## GoLite language specification
- [Lexical and syntactic rules](https://github.com/EmolLi/GoLite/blob/master/doc/Milestone1_Specifications.pdf) 
- [Semantic rules](https://github.com/EmolLi/GoLite/blob/master/doc/Milestone2_Specifications.pdf)


## Project Structure
* `grading`: Test programs are organized by compilation phase and by expected result. Valid programs should output `OK` and status code `0`, while invalid programs should output `Error: <description>` and status code `1`.
  * `Scan+parse`: Runs both the scanner and parser phases
  * `Typecheck`: Runs until the end of the typechecker phase
  * `Semantics+codegen`: Runs until the compiler outputs the target code
  * `Extra+codegen`: Runs until the compiler outputs the target code, focuses on testing execution order
  * `Benchmarks+codegen`: Runs until the compiler outputs the target code, focuses on testing the performance of generated code
* `doc`: Document for the project
  * `Milestone1_Specifications`: Lexical and syntactic rules of GoLite
  * `Milestone2_Specifications`: Semantic rules of GoLite
  * `FinalReport`: Report for this project
* `src`: Source code for the compiler
* `build.sh`: Build script for the compiler
* `run.sh`: Runs the compiler using two arguments (mode - $1 and input file - $2). 
* `test.sh`: Automatically runs the compiler against test programs in the programs directory and checks the output
* `grading.sh`: Grading file for the compiler, can be used for testing purpose
