.ORIG x3000

ADD R0, R1, #1
STR R0, R6, #0
ST R1, DEAD

    DEAD  .FILL xFF
    PROMPT_INVALID_MESSAGE  .STRINGZ    " is not a valid input.\n\n"
.END