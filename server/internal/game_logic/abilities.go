package gamelogic

type PlayerAbilities interface {
	GetBombCapacity() uint32
	GetBlastRange() uint32
	GetMovementIntervalTicks() uint32
}

type BasePlayerAbilities struct {
	bombCapacity          uint32
	blastRange            uint32
	movementIntervalTicks uint32
}

func NewBasePlayerAbilities(
	bombCapacity uint32,
	blastRange uint32,
	movementIntervalTicks uint32,
) *BasePlayerAbilities {
	return &BasePlayerAbilities{
		bombCapacity:          bombCapacity,
		blastRange:            blastRange,
		movementIntervalTicks: movementIntervalTicks,
	}
}

func (a *BasePlayerAbilities) GetBombCapacity() uint32 {
	return a.bombCapacity
}

func (a *BasePlayerAbilities) GetBlastRange() uint32 {
	return a.blastRange
}

func (a *BasePlayerAbilities) GetMovementIntervalTicks() uint32 {
	return a.movementIntervalTicks
}

type PlayerAbilitiesDecorator struct {
	abilities PlayerAbilities
}

func (d *PlayerAbilitiesDecorator) GetBombCapacity() uint32 {
	return d.abilities.GetBombCapacity()
}

func (d *PlayerAbilitiesDecorator) GetBlastRange() uint32 {
	return d.abilities.GetBlastRange()
}

func (d *PlayerAbilitiesDecorator) GetMovementIntervalTicks() uint32 {
	return d.abilities.GetMovementIntervalTicks()
}

type BombCapacityDecorator struct {
	PlayerAbilitiesDecorator
	bonus uint32
}

func NewBombCapacityDecorator(
	abilities PlayerAbilities,
	bonus uint32,
) *BombCapacityDecorator {
	return &BombCapacityDecorator{
		PlayerAbilitiesDecorator: PlayerAbilitiesDecorator{
			abilities: abilities,
		},
		bonus: bonus,
	}
}

func (d *BombCapacityDecorator) GetBombCapacity() uint32 {
	return d.abilities.GetBombCapacity() + d.bonus
}

type BlastRangeDecorator struct {
	PlayerAbilitiesDecorator
	bonus uint32
}

func NewBlastRangeDecorator(
	abilities PlayerAbilities,
	bonus uint32,
) *BlastRangeDecorator {
	return &BlastRangeDecorator{
		PlayerAbilitiesDecorator: PlayerAbilitiesDecorator{
			abilities: abilities,
		},
		bonus: bonus,
	}
}

func (d *BlastRangeDecorator) GetBlastRange() uint32 {
	return d.abilities.GetBlastRange() + d.bonus
}

type FasterMovementDecorator struct {
	PlayerAbilitiesDecorator
	reduction uint32
}

func NewFasterMovementDecorator(
	abilities PlayerAbilities,
	reduction uint32,
) *FasterMovementDecorator {
	return &FasterMovementDecorator{
		PlayerAbilitiesDecorator: PlayerAbilitiesDecorator{
			abilities: abilities,
		},
		reduction: reduction,
	}
}

func (d *FasterMovementDecorator) GetMovementIntervalTicks() uint32 {
	interval := d.abilities.GetMovementIntervalTicks()

	if d.reduction >= interval {
		return 1
	}

	return interval - d.reduction
}