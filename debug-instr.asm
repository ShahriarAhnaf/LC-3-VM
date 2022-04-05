.ORIG x3000

STR R0, R6, #0
ST R1, DEAD

    DEAD  .FILL xFF
    PROMPT_INVALID_MESSAGE  .STRINGZ    " is not a valid input.\n\n"
.END