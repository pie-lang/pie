module Ast where

type ModuleName

data ImportStatment = ImportStatment name

data Statment = ModuleDeclareStatment
	| ImportStatment

data Node = Empty | Node Name [Node]
