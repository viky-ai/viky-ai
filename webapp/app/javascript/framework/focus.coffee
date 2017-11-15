class FocusInput

  @atStart: (selector) ->
    input = $(selector)[0];
    input.focus();

  @atEnd: (selector) ->
    input = $(selector)[0];
    input.selectionStart = input.value.length;
    input.selectionEnd = input.value.length;
    input.focus();

module.exports = FocusInput
