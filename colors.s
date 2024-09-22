          * = $0000
          LDA #$10
          STA $00
          LDA #$FF
          STA $01
          LDA #$01
          STA $02
          LDA $02
          STA ($00),Y
          LDA $00
          CMP #$FF
          BEQ L0023
          INC $00
          INC $02
          LDA $02
          AND #$03
          STA $02
          JMP $060C
L0023     BRK
          .END
