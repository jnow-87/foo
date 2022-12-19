#
# Copyright (C) 2022 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# foreground colors
term_fg_black := "\033[30m"
term_fg_red := "\033[31m"
term_fg_green := "\033[32m"
term_fg_yellow := "\033[33m"
term_fg_blue := "\033[34m"
term_fg_violet := "\033[35m"
term_fg_kobalt := "\033[36m"
term_fg_white := "\033[37m"

# background colors
term_bg_black := "\033[40m"
term_bg_red := "\033[41m"
term_bg_green := "\033[42m"
term_bg_yellow := "\033[43m"
term_bg_blue := "\033[44m"
term_bg_violet := "\033[45m"
term_bg_kobalt := "\033[46m"
term_bg_white := "\033[47m"

# modes
term_bold := "\033[1m"
term_underline := "\033[4m"
term_blink := "\033[5m"
term_inverse := "\033[7m"
term_invisible := "\033[8m"

term_reset_attr := "\033[0m"


# print text with terminal mode
#
#	$(call mode,<mode-list>,<text>)
define term_mode
$(subst $(space),,$(foreach mode,$(1),$(term_$(mode))))$(2)$(term_reset_attr)
endef

# print text with foreground color
#
#	$(call fg,<color>,<text>)
define fg
$(call term_mode,fg_$(1),$(2))
endef

# print text with background color
#
#	$(call bg,<color>,<text>)
define bg
$(call term_mode,bg_$(1),$(2))
endef
