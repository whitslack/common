#pragma once

#define EMPTY()
#define DEFER(...) __VA_ARGS__ EMPTY()
#define OBSTRUCT(...) __VA_ARGS__ DEFER(EMPTY)()
#define EXPAND(...) __VA_ARGS__

#define COMMA DEFER(COMMA_)()
#define COMMA_() ,
#define LPAREN DEFER(LPAREN_)()
#define LPAREN_() (
#define RPAREN DEFER(RPAREN_)()
#define RPAREN_() )

#define CAT(...) CAT_(__VA_ARGS__)
#define CAT_(_, ...) _ ## __VA_ARGS__
#define TAC(...) TAC_(__VA_ARGS__)
#define TAC_(_, ...) __VA_ARGS__ ## _

// FIRST( A, B, C ) => A
#define FIRST(...) FIRST_(__VA_ARGS__)
#define FIRST_(_, ...) _

// REST( A, B, C ) => B, C
#define REST(...) REST_(__VA_ARGS__)
#define REST_(_, ...) __VA_ARGS__

// HEAD( (A) (B) (C) ) => A
// HEAD( (A,0) (B,1) (C,2) ) => A,0
// HEAD( ((A)(0)) ((B)(1)) ((C)(2)) ) => (A)(0)
#define HEAD(seq) TAC(_, HEAD_0 seq)
#define HEAD_0(...) __VA_ARGS__ HEAD_1
#define HEAD_1(...) HEAD_2
#define HEAD_2(...) HEAD_1
#define HEAD_1_
#define HEAD_2_

// TAIL( (A) (B) (C) ) => (B) (C)
// TAIL( (A,0) (B,1) (C,2) ) => (B,1) (C,2)
// TAIL( ((A)(0)) ((B)(1)) ((C)(2)) ) => ((B)(1)) ((C)(2))
#define TAIL(seq) TAC(_, TAIL_0 seq)
#define TAIL_0(...) TAIL_1
#define TAIL_1(...) (__VA_ARGS__) TAIL_2
#define TAIL_2(...) (__VA_ARGS__) TAIL_1
#define TAIL_1_
#define TAIL_2_

// COLLAPSE( (A) (B) (C) ) => A B C
#define COLLAPSE(seq) TAC(_, COLLAPSE_0 seq)
#define COLLAPSE_0(...) __VA_ARGS__ COLLAPSE_1
#define COLLAPSE_1(...) __VA_ARGS__ COLLAPSE_0
#define COLLAPSE_0_
#define COLLAPSE_1_

// GLUE( (A) (B) (C) ) => A B C
// GLUE( (A) (B) (C), ) => A B C
// GLUE( (A) (B) (C), , ) => A , B , C
// GLUE( (A) (B) (C), : ) => A : B : C
#define GLUE(seq, ...) COLLAPSE(TAIL(REST(EXPAND(TAC(_, GLUE_0 seq) (__VA_ARGS__), TAC(_, CLOSE_0 seq)))))
#define GLUE_0(...) GLUE_F LPAREN (__VA_ARGS__), GLUE_1
#define GLUE_1(...) GLUE_F LPAREN (__VA_ARGS__), GLUE_0
#define GLUE_0_
#define GLUE_1_
#define GLUE_F(...) GLUE_F_(__VA_ARGS__)
#define GLUE_F_(arg, glue, ...) glue, glue arg __VA_ARGS__

// LIST( (A) (B) (C) ) => A , B , C
#define LIST(seq) GLUE(seq, ,)

// REVERSE( (A) (B) (C) ) => (C) (B) (A)
#define REVERSE(seq) EXPAND(TAC(_, REVERSE_0 seq) TAC(_, CLOSE_0 seq))
#define REVERSE_0(...) REVERSE_F LPAREN (__VA_ARGS__), REVERSE_1
#define REVERSE_1(...) REVERSE_F LPAREN (__VA_ARGS__), REVERSE_0
#define REVERSE_0_
#define REVERSE_1_
#define REVERSE_F(_0, _1) _1 _0

// #define INC(x) (x)+1
// MAP( (A) (B) (C), INC ) => ((A)+1) ((B)+1) ((C)+1)
// #define STRINGIZE(x) #x
// MAP( (A) (B) (C), STRINGIZE ) => ("A") ("B") ("C")
#define MAP(seq, op) REST(EXPAND(TAC(_, MAP_0 seq) op, TAC(_, CLOSE_0 seq)))
#define MAP_0(...) MAP_F LPAREN (__VA_ARGS__), MAP_1
#define MAP_1(...) MAP_F LPAREN (__VA_ARGS__), MAP_0
#define MAP_0_
#define MAP_1_
#define MAP_F(...) MAP_F_(__VA_ARGS__)
#define MAP_F_(arg, op, ...) op, (op arg) __VA_ARGS__

#define CLOSE_0(...) RPAREN CLOSE_1
#define CLOSE_1(...) RPAREN CLOSE_0
#define CLOSE_0_
#define CLOSE_1_
