if (CONFIG_PREFIX MATCHES $ENV{HOME})
set (CONFIG_LV2DIR ${CONFIG_PREFIX}/.lv2)
endif ()
install (FILES qmidiarp_lfo.ttl DESTINATION ${CONFIG_LV2DIR}/qmidiarp_lfo.lv2/)
install (FILES qmidiarp_lfo_ui.ttl DESTINATION ${CONFIG_LV2DIR}/qmidiarp_lfo.lv2/)
install (FILES manifest.ttl DESTINATION ${CONFIG_LV2DIR}/qmidiarp_lfo.lv2/)

