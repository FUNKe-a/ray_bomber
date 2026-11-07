package gamelogic

func applyAbilitiesDemo(abilities PlayerAbilities) PlayerAbilities {
	abilities = NewBombCapacityDecorator(abilities, 1)
	abilities = NewBlastRangeDecorator(abilities, 1)
	abilities = NewFasterMovementDecorator(abilities, 1)

	return abilities
}
